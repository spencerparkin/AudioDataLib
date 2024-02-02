#include "SampleBasedSynth.h"
#include "SoundFontData.h"
#include "LoopedAudioModule.h"
#include "PitchShiftModule.h"
#include "AttenuationModule.h"
#include "MidiData.h"
#include "MixerModule.h"
#include "Function.h"
#include "Error.h"

using namespace AudioDataLib;

SampleBasedSynth::SampleBasedSynth(bool ownsAudioStream, bool ownsSoundFontData) : MidiSynth(ownsAudioStream)
{
	this->ownsSoundFontData = ownsSoundFontData;
	this->soundFontMap = new SoundFontMap();
	this->noteMap = new NoteMap();
	this->mixerModuleLeftEar = new MixerModule();
	this->mixerModuleRightEar = new MixerModule();
}

/*virtual*/ SampleBasedSynth::~SampleBasedSynth()
{
	this->Clear();

	delete this->soundFontMap;
	delete this->noteMap;
	delete this->mixerModuleLeftEar;
	delete this->mixerModuleRightEar;
}

/*virtual*/ bool SampleBasedSynth::GenerateAudio(Error& error)
{
	this->mixerModuleLeftEar->PruneDeadBranches();
	this->mixerModuleRightEar->PruneDeadBranches();

	return MidiSynth::GenerateAudio(error);
}

/*virtual*/ bool SampleBasedSynth::ReceiveMessage(double deltaTimeSeconds, const uint8_t* message, uint64_t messageSize, Error& error)
{
	MidiData::ChannelEvent channelEvent;
	ReadOnlyBufferStream bufferStream(message, messageSize);
	if (!channelEvent.Decode(bufferStream, error))
	{
		// TODO: For now we only handle channel events.  Handle other events too.
		return true;
	}

	SoundFontData* soundFontData = this->GetSoundFontData(channelEvent.channel);
	if (!soundFontData)
	{
		error.Add(FormatString("No sound font loaded for MIDI channel %d.", channelEvent.channel));
		return false;
	}

	switch (channelEvent.type)
	{
		case MidiData::ChannelEvent::NOTE_ON:
		{
			uint8_t pitchValue = channelEvent.param1;
			uint8_t velocityValue = channelEvent.param2;

			NoteMap::iterator iter = this->noteMap->find(pitchValue);
			if (iter != this->noteMap->end())
			{
				// TODO: For a single pitch value, is it possible to get a NOTE_ON message twice without a NOTE_OFF message inbetween?
				//       If so, then rather than treat this as an error, we should just cancel the existing note?
				error.Add("Pitch value already in note map?");
				return false;
			}

			double noteFrequency = this->MidiPitchToFrequency(pitchValue);
			double noteVolume = this->MidiVelocityToAmplitude(velocityValue);

			const SoundFontData::AudioSample* audioSample = soundFontData->FindRelevantAudioSample(pitchValue, velocityValue);
			if (!audioSample)
			{
				audioSample = soundFontData->FindClosestAudioSample(noteFrequency, noteVolume);
				if (!audioSample)
				{
					error.Add(FormatString("Failed to find audio sample for pitch %f and volume %f.", noteFrequency, noteVolume));
					return false;
				}

				fprintf(stderr, "Warning: Had to look for closest audio sample rather than the one prescribed by the sound-font data.\n");
			}

			const SoundFontData::LoopedAudioData* leftAudioData = audioSample->FindLoopedAudioData(SoundFontData::LoopedAudioData::ChannelType::LEFT_EAR);
			const SoundFontData::LoopedAudioData* rightAudioData = audioSample->FindLoopedAudioData(SoundFontData::LoopedAudioData::ChannelType::RIGHT_EAR);

			if (!leftAudioData || !rightAudioData)
			{
				error.Add(FormatString("Could not get left and right audio data for pitch %f.", noteFrequency));
				return false;
			}

			auto loopedAudioModuleLeft = new LoopedAudioModule();
			auto loopedAudioModuleRight = new LoopedAudioModule();

			auto pitchShiftModuleLeft = new PitchShiftModule();
			auto pitchShiftModuleRight = new PitchShiftModule();

			auto attenuationModuleLeft = new AttenuationModule();
			auto attenuationModuleRight = new AttenuationModule();

			Note note;

			note.leftModuleID = this->mixerModuleLeftEar->AddModule(attenuationModuleLeft);
			note.rightModuleID = this->mixerModuleRightEar->AddModule(attenuationModuleRight);

			this->noteMap->insert(std::pair<uint8_t, Note>(pitchValue, note));

			attenuationModuleLeft->SetDependentModule(pitchShiftModuleLeft);
			attenuationModuleRight->SetDependentModule(pitchShiftModuleRight);

			pitchShiftModuleLeft->SetDependentModule(loopedAudioModuleLeft);
			pitchShiftModuleRight->SetDependentModule(loopedAudioModuleRight);

			if (!loopedAudioModuleLeft->UseLoopedAudioData(leftAudioData, 0, error))
				return false;

			if (!loopedAudioModuleRight->UseLoopedAudioData(rightAudioData, 0, error))
				return false;

			// TODO: There is a bug where a piano note left on will decay, but then play again before you've lifted the key.
			//       I think that I need to review the looping rules and know when a sound sample is supposed to be done, at
			//       which point, it should get pruned automatically even if there is no off-event given.

			pitchShiftModuleLeft->SetSourceAndTargetFrequencies(leftAudioData->GetMetaData()->analyticalPitch, noteFrequency);
			pitchShiftModuleRight->SetSourceAndTargetFrequencies(rightAudioData->GetMetaData()->analyticalPitch, noteFrequency);

			break;
		}
		case MidiData::ChannelEvent::NOTE_OFF:
		{
			uint8_t pitchValue = channelEvent.param1;

			NoteMap::iterator iter = this->noteMap->find(pitchValue);
			if (iter != this->noteMap->end())
			{
				const Note& note = iter->second;

				auto attenuationModuleLeft = dynamic_cast<AttenuationModule*>(this->mixerModuleLeftEar->FindModule(note.leftModuleID));
				auto attenuationModuleRight = dynamic_cast<AttenuationModule*>(this->mixerModuleRightEar->FindModule(note.rightModuleID));

				if (attenuationModuleLeft)
				{
					attenuationModuleLeft->SetAttenuationFunction(new LinearFallOffFunction(0.2));
					attenuationModuleLeft->TriggerFallOff();
				}

				if (attenuationModuleRight)
				{
					attenuationModuleRight->SetAttenuationFunction(new LinearFallOffFunction(0.2));
					attenuationModuleRight->TriggerFallOff();
				}

				this->noteMap->erase(iter);
			}

			break;
		}
	}

	return true;
}

/*virtual*/ SynthModule* SampleBasedSynth::GetRootModule(uint16_t channel)
{
	switch (channel)
	{
	case 0:
		return this->mixerModuleLeftEar;
	case 1:
		return this->mixerModuleRightEar;
	default:
		return nullptr;
	}
}

void SampleBasedSynth::SetSoundFontData(uint16_t channel, SoundFontData* soundFontData)
{
	SoundFontMap::iterator iter = this->soundFontMap->find(channel);
	if (iter != this->soundFontMap->end())
	{
		if (this->ownsSoundFontData)
			delete iter->second;
		this->soundFontMap->erase(iter);
	}

	Error error;
	for (uint32_t i = 0; i < soundFontData->GetNumAudioSamples(); i++)
	{
		const SoundFontData::AudioSample* audioSample = soundFontData->GetAudioSample(i);
		for (uint32_t j = 0; j < audioSample->GetNumLoopedAudioDatas(); j++)
		{
			const SoundFontData::LoopedAudioData* audioData = audioSample->GetLoopedAudioData(j);
			
			printf("Analyzing %s... ", audioData->GetName().c_str());
			const SoundFontData::LoopedAudioData::MetaData* metaData = audioData->GetMetaData();
			printf("Estimated Pitch: %f Hz\n", metaData->analyticalPitch);

			audioData->GetCachedWaveForm(0, error);
		}
	}

	this->soundFontMap->insert(std::pair<uint16_t, SoundFontData*>(channel, soundFontData));
}

SoundFontData* SampleBasedSynth::GetSoundFontData(uint16_t channel)
{
	SoundFontMap::iterator iter = this->soundFontMap->find(channel);
	if (iter == this->soundFontMap->end())
		return nullptr;

	return iter->second;
}

void SampleBasedSynth::Clear()
{
	if (this->ownsSoundFontData)
		for (auto pair : *this->soundFontMap)
			delete pair.second;

	this->soundFontMap->clear();
	this->noteMap->clear();
}