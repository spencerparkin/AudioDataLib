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

			printf("%1.3f Hz <== (L: %1.3f Hz; R: %1.3f Hz)\n",
								noteFrequency,
								leftAudioData->GetMetaData().pitch,
								rightAudioData->GetMetaData().pitch);
			printf("L: %s\n", leftAudioData->GetName().c_str());
			printf("R: %s\n", rightAudioData->GetName().c_str());

			pitchShiftModuleLeft->SetSourceAndTargetFrequencies(leftAudioData->GetMetaData().pitch, noteFrequency);
			pitchShiftModuleRight->SetSourceAndTargetFrequencies(rightAudioData->GetMetaData().pitch, noteFrequency);

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

bool SampleBasedSynth::SetSoundFontData(uint16_t channel, SoundFontData* soundFontData, bool estimateFrequencies, Error& error)
{
	SoundFontMap::iterator iter = this->soundFontMap->find(channel);
	if (iter != this->soundFontMap->end())
	{
		if (this->ownsSoundFontData)
			delete iter->second;
		this->soundFontMap->erase(iter);
	}

	for (uint32_t i = 0; i < soundFontData->GetNumAudioSamples(); i++)
	{
		const SoundFontData::AudioSample* audioSample = soundFontData->GetAudioSample(i);
		for (uint32_t j = 0; j < audioSample->GetNumLoopedAudioDatas(); j++)
		{
			const SoundFontData::LoopedAudioData* audioData = audioSample->GetLoopedAudioData(j);

			if (estimateFrequencies)
			{
				// This option really only exists because for a long time, I didn't know that the SF2 file
				// format embedded the frequencies of its samples.  I thought it was dumb that it didn't
				// until I finally found out that it actually does.  In any case, learning to estimate the
				// fundamental pitch of an audio sample was not a bad thing, so I guess it all worked out
				// for the better anyway.  This estimate, however, is not always accurate, especially in
				// the high upper or very low pitch ranges.
				printf("Analyzing %s... ", audioData->GetName().c_str());
				
				if (!audioData->CalcMetaData(error))
					return false;

				printf("Estimated Pitch: %f Hz\n", audioData->GetMetaData().pitch);
			}
			else
			{
				SoundFontData::LoopedAudioData::MetaData metaData = audioData->GetMetaData();
				metaData.pitch = MidiSynth::MidiPitchToFrequency(audioData->GetMidiKeyInfo().overridingRoot);
				audioData->SetMetaData(metaData);
			}

			audioData->GetCachedWaveForm(0, error);
		}
	}

	this->soundFontMap->insert(std::pair<uint16_t, SoundFontData*>(channel, soundFontData));

	return true;
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