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
	this->mixerModuleLeftEar = new MixerModule();
	this->mixerModuleRightEar = new MixerModule();
}

/*virtual*/ SampleBasedSynth::~SampleBasedSynth()
{
	this->Clear();

	delete this->soundFontMap;
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

			double noteFrequency = this->MidiPitchToFrequency(pitchValue);
			double noteVolume = this->MidiVelocityToAmplitude(velocityValue);

			printf("Frequency: %f Hz\n", noteFrequency);

			const SoundFontData::AudioSample* audioSample = soundFontData->FindRelevantAudioSample(pitchValue, velocityValue);
			if (!audioSample)
			{
				audioSample = soundFontData->FindClosestAudioSample(noteFrequency, noteVolume);
				if (!audioSample)
				{
					error.Add(FormatString("Failed to find audio sample for pitch %f and volume %f.", noteFrequency, noteVolume));
					return false;
				}
			}

			printf("Audio sample frequency: %f Hz\n", audioSample->GetLoopedAudioData(0)->GetMetaData()->analyticalPitch);
			printf("Audio sample name: %s\n", audioSample->GetLoopedAudioData(0)->GetName().c_str());

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

			// TODO: There is a bug here.  If you trill, sometimes a note is created of the same pitch
			//       as one that has not yet faded out, and so what we do here is cut that note off
			//       prematurely.  I think we need to change how the mixer works as a container.  I
			//       think the caller should have to maintain a handle to the added module for later
			//       reference.  The mixer just maintains a list, not a map.  We will handle the map.
			//       Modules in the list of the mixer can fade away and die as needed.
			this->mixerModuleLeftEar->SetModule(pitchValue, attenuationModuleLeft);
			this->mixerModuleRightEar->SetModule(pitchValue, attenuationModuleRight);

			attenuationModuleLeft->SetDependentModule(pitchShiftModuleLeft);
			attenuationModuleRight->SetDependentModule(pitchShiftModuleRight);

			pitchShiftModuleLeft->SetDependentModule(loopedAudioModuleLeft);
			pitchShiftModuleRight->SetDependentModule(loopedAudioModuleRight);

			// This seems fast enough, but we could benefit from some caching here.
			if (!loopedAudioModuleLeft->UseLoopedAudioData(leftAudioData, 0, error))
				return false;

			if (!loopedAudioModuleRight->UseLoopedAudioData(rightAudioData, 0, error))
				return false;

			pitchShiftModuleLeft->SetSourceAndTargetFrequencies(leftAudioData->GetMetaData()->analyticalPitch, noteFrequency);
			pitchShiftModuleRight->SetSourceAndTargetFrequencies(rightAudioData->GetMetaData()->analyticalPitch, noteFrequency);

			break;
		}
		case MidiData::ChannelEvent::NOTE_OFF:
		{
			uint8_t pitchValue = channelEvent.param1;

			auto attenuationModuleLeft = dynamic_cast<AttenuationModule*>(this->mixerModuleLeftEar->GetModule(pitchValue));
			auto attenuationModuleRight = dynamic_cast<AttenuationModule*>(this->mixerModuleRightEar->GetModule(pitchValue));

			if (!attenuationModuleLeft || !attenuationModuleRight)
			{
				error.Add(FormatString("Failed to find attenuation modules for pitch %d.", pitchValue));
				return false;
			}

			attenuationModuleLeft->SetAttenuationFunction(new LinearFallOffFunction(0.2));
			attenuationModuleRight->SetAttenuationFunction(new LinearFallOffFunction(0.2));

			attenuationModuleLeft->TriggerFallOff();
			attenuationModuleRight->TriggerFallOff();

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
			audioData->GetMetaData();		// We have to warm this cache before the synth starts up.
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
}