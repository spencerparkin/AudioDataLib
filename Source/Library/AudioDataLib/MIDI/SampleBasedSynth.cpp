#include "AudioDataLib/MIDI/SampleBasedSynth.h"
#include "AudioDataLib/SynthModules/LoopedAudioModule.h"
#include "AudioDataLib/SynthModules/PitchShiftModule.h"
#include "AudioDataLib/SynthModules/AttenuationModule.h"
#include "AudioDataLib/SynthModules/ReverbModule.h"
#include "AudioDataLib/SynthModules/DuplicationModule.h"
#include "AudioDataLib/SynthModules/DelayModule.h"
#include "AudioDataLib/FileDatas/MidiData.h"
#include "AudioDataLib/SynthModules/MixerModule.h"
#include "AudioDataLib/Math/Function.h"
#include "AudioDataLib/ErrorSystem.h"

using namespace AudioDataLib;

SampleBasedSynth::SampleBasedSynth()
{
	this->reverbEnabled = false;
	this->estimateFrequencies = false;
	this->waveTableData = new std::shared_ptr<WaveTableData>();
	this->channelMap = new ChannelMap();
	this->noteMap = new NoteMap();
	this->leftEarRootModule = new std::shared_ptr<SynthModule>();
	this->rightEarRootModule = new std::shared_ptr<SynthModule>();

	this->SetReverbEnabled(false);
}

/*virtual*/ SampleBasedSynth::~SampleBasedSynth()
{
	this->Clear();

	delete this->waveTableData;
	delete this->channelMap;
	delete this->noteMap;
	delete this->leftEarRootModule;
	delete this->rightEarRootModule;
}

/*virtual*/ bool SampleBasedSynth::Process()
{
	MixerModule* leftMixerModule = (*this->leftEarRootModule)->FindModule<MixerModule>();
	leftMixerModule->PruneDeadBranches();

	MixerModule* rightMixerModule = (*this->rightEarRootModule)->FindModule<MixerModule>();
	rightMixerModule->PruneDeadBranches();

	return MidiSynth::Process();
}

/*virtual*/ bool SampleBasedSynth::ReceiveMessage(double deltaTimeSeconds, const uint8_t* message, uint64_t messageSize)
{
	if (!this->waveTableData->get())
	{
		ErrorSystem::Get()->Add("No wave-table set!");
		return false;
	}

	MidiData::ChannelEvent channelEvent;
	ReadOnlyBufferStream bufferStream(message, messageSize);
	if (!channelEvent.Decode(bufferStream))
	{
		// TODO: For now we only handle channel events.  Handle other events too.
		return true;
	}

	uint8_t instrument = 0;
	if (!this->GetChannelInstrument(channelEvent.channel + 1, instrument))
	{
		ErrorSystem::Get()->Add(std::format("Could not get instrument for channel {}.", channelEvent.channel));
		return false;
	}

	switch (channelEvent.type)
	{
		case MidiData::ChannelEvent::PROGRAM_CHANGE:
		{
			uint16_t instrument = channelEvent.param1;
			if (!this->SetChannelInstrument(channelEvent.channel + 1, instrument + 1))
				return false;
			break;
		}
		case MidiData::ChannelEvent::NOTE_ON:
		{
			uint8_t pitchValue = channelEvent.param1;
			uint8_t velocityValue = channelEvent.param2;

			NoteMap::iterator iter = this->noteMap->find(pitchValue);
			if (iter != this->noteMap->end())
			{
				// TODO: For a single pitch value, is it possible to get a NOTE_ON message twice without a NOTE_OFF message inbetween?
				//       If so, then rather than treat this as an error, we should just cancel the existing note?
				ErrorSystem::Get()->Add("Pitch value already in note map?");
				return false;
			}

			double noteFrequency = this->MidiPitchToFrequency(pitchValue);
			double noteVolume = this->MidiVelocityToAmplitude(velocityValue);

			const WaveTableData::AudioSampleData* audioSampleData = (*this->waveTableData)->FindAudioSample(instrument, pitchValue, velocityValue);
			if (!audioSampleData)
			{
				ErrorSystem::Get()->Add(std::format("Failed to find audio sample for pitch {} ({}) and volume {} ({}).", pitchValue, noteFrequency, velocityValue, noteVolume));
				return false;
			}

			Note note;

			if (!this->GenerateModuleGraph(audioSampleData, noteFrequency, note.leftEarModule))
				return false;

			if(!this->reverbEnabled)
				if (!this->GenerateModuleGraph(audioSampleData, noteFrequency, note.rightEarModule))
					return false;

			this->noteMap->insert(std::pair<uint8_t, Note>(pitchValue, note));

			MixerModule* leftMixerModule = (*this->leftEarRootModule)->FindModule<MixerModule>();
			leftMixerModule->AddDependentModule(note.leftEarModule);

			MixerModule* rightMixerModule = (*this->rightEarRootModule)->FindModule<MixerModule>();
			if(!this->reverbEnabled)
				rightMixerModule->AddDependentModule(note.rightEarModule);
			else
			{
				if (rightMixerModule != leftMixerModule)
				{
					ErrorSystem::Get()->Add("Expected only one mixer module in the reverb case!");
					return false;
				}
			}

			break;
		}
		case MidiData::ChannelEvent::NOTE_OFF:
		{
			uint8_t pitchValue = channelEvent.param1;

			NoteMap::iterator iter = this->noteMap->find(pitchValue);
			if (iter != this->noteMap->end())
			{
				const Note& note = iter->second;

				// Some notes die before a key is released, while others die some time after the key is released.

				if (note.leftEarModule.get())
				{
					auto attenuationModuleLeft = note.leftEarModule->FindModule<AttenuationModule>();
					if (attenuationModuleLeft)
					{
						attenuationModuleLeft->SetAttenuationFunction(new LinearFallOffFunction(0.05));
						attenuationModuleLeft->TriggerFallOff();
					}
				}

				if (note.rightEarModule.get())
				{
					auto attenuationModuleRight = note.rightEarModule->FindModule<AttenuationModule>();
					if (attenuationModuleRight)
					{
						attenuationModuleRight->SetAttenuationFunction(new LinearFallOffFunction(0.05));
						attenuationModuleRight->TriggerFallOff();
					}
				}

				this->noteMap->erase(iter);
			}

			break;
		}
	}

	return true;
}

bool SampleBasedSynth::GenerateModuleGraph(const WaveTableData::AudioSampleData* audioSampleData, double noteFrequency, std::shared_ptr<SynthModule>& synthModule)
{
	auto loopedAudioModule = new LoopedAudioModule();
	if (!loopedAudioModule->UseLoopedAudioData(audioSampleData, 0))
	{
		delete loopedAudioModule;
		return false;
	}

	auto pitchShiftModule = new PitchShiftModule();
	pitchShiftModule->SetSourceAndTargetFrequencies(audioSampleData->GetMetaData().pitch, noteFrequency);
	pitchShiftModule->AddDependentModule(std::shared_ptr<SynthModule>(loopedAudioModule));

	auto attenuationModule = new AttenuationModule();
	attenuationModule->AddDependentModule(std::shared_ptr<SynthModule>(pitchShiftModule));
	synthModule.reset(attenuationModule);

	return true;
}

/*virtual*/ SynthModule* SampleBasedSynth::GetRootModule(uint16_t channel)
{
	switch (channel)
	{
	case 0:
		return this->leftEarRootModule->get();
	case 1:
		return this->rightEarRootModule->get();
	default:
		return nullptr;
	}
}

void SampleBasedSynth::SetReverbEnabled(bool reverbEnabled)
{
	this->reverbEnabled = reverbEnabled;

	this->leftEarRootModule->reset();
	this->rightEarRootModule->reset();

	if (this->reverbEnabled)
	{
		auto leftDelayModule = new DelayModule();
		auto rightDelayModule = new DelayModule();

		leftDelayModule->SetDelay(0.0);
		rightDelayModule->SetDelay(12.0 / 1000.0);

		this->leftEarRootModule->reset(leftDelayModule);
		this->rightEarRootModule->reset(rightDelayModule);

		auto mixerModule = new MixerModule();

		auto reverbModule = new ReverbModule(0);
		reverbModule->AddDependentModule(std::shared_ptr<SynthModule>(mixerModule));

		std::shared_ptr<SynthModule> duplicationModule(new DuplicationModule());
		duplicationModule->AddDependentModule(std::shared_ptr<SynthModule>(reverbModule));

		leftDelayModule->AddDependentModule(duplicationModule);
		rightDelayModule->AddDependentModule(duplicationModule);
	}
	else
	{
		this->leftEarRootModule->reset(new MixerModule());
		this->rightEarRootModule->reset(new MixerModule());
	}
}

/*virtual*/ bool SampleBasedSynth::Initialize()
{
	if (!this->waveTableData->get())
	{
		ErrorSystem::Get()->Add("No wave-table set!");
		return false;
	}

	for (uint32_t i = 0; i < (*this->waveTableData)->GetNumAudioSamples(); i++)
	{
		auto audioSampleData = dynamic_cast<const WaveTableData::AudioSampleData*>((*this->waveTableData)->GetAudioSample(i));
		if (!audioSampleData)
			continue;

		if (this->estimateFrequencies)
		{
			// This option really only exists because for a long time, I didn't know that the SF2 file
			// format embedded the frequencies of its samples.  I thought it was dumb that it didn't
			// until I finally found out that it actually does.  In any case, learning to estimate the
			// fundamental pitch of an audio sample was not a bad thing, so I guess it all worked out
			// for the better anyway.  This estimate, however, is not always accurate, especially in
			// the high upper or very low pitch ranges.
			printf("Analyzing %s... ", audioSampleData->GetName().c_str());

			if (!audioSampleData->CalcMetaData())
				return false;

			printf("Estimated Pitch: %f Hz\n", audioSampleData->GetMetaData().pitch);
			printf("Estimated Volume: %f\n", audioSampleData->GetMetaData().volume);	// TODO: What are the units?  dB?
		}
		else
		{
			WaveTableData::AudioSampleData::MetaData metaData = audioSampleData->GetMetaData();
			WaveTableData::AudioSampleData::Character character = audioSampleData->GetCharacter();
			metaData.pitch = MidiSynth::MidiPitchToFrequency(character.originalPitch);
			metaData.pitch = MidiSynth::TunePitch(metaData.pitch, character.fineTuneCents);
			audioSampleData->SetMetaData(metaData);
		}

		audioSampleData->GetCachedWaveForm(0);
	}

	return true;
}

bool SampleBasedSynth::SetChannelInstrument(uint8_t channel, uint8_t instrument)
{
	if (!(1 <= channel && channel <= 16))
	{
		ErrorSystem::Get()->Add(std::format("Channel number ({}) out of range [1,16].", channel));
		return false;
	}

	ChannelMap::iterator iter = this->channelMap->find(channel);
	if (iter != this->channelMap->end())
		this->channelMap->erase(iter);

	this->channelMap->insert(std::pair<uint8_t, uint8_t>(channel, instrument));
	return true;
}

bool SampleBasedSynth::GetChannelInstrument(uint8_t channel, uint8_t& instrument) const
{
	instrument = 0;

	ChannelMap::iterator iter = this->channelMap->find(channel);
	if (iter == this->channelMap->end())
		return false;

	instrument = iter->second;
	return true;
}

void SampleBasedSynth::Clear()
{
	this->waveTableData->reset();
	this->channelMap->clear();
	this->noteMap->clear();
}