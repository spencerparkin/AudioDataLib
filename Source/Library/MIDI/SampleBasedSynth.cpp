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

			const SoundFontData::PitchData* pitchData = soundFontData->FindClosestPitchData(noteFrequency, noteVolume);
			if (!pitchData)
			{
				error.Add(FormatString("Failed to find pitch data for pitch %f and volume %f.", noteFrequency, noteVolume));
				return false;
			}

			const SoundFontData::LoopedAudioData* leftAudioData = pitchData->FindLoopedAudioData(SoundFontData::LoopedAudioData::ChannelType::LEFT_EAR);
			const SoundFontData::LoopedAudioData* rightAudioData = pitchData->FindLoopedAudioData(SoundFontData::LoopedAudioData::ChannelType::RIGHT_EAR);

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

			this->mixerModuleLeftEar->SetModule(pitchValue, attenuationModuleLeft);
			this->mixerModuleRightEar->SetModule(pitchValue, attenuationModuleRight);

			attenuationModuleLeft->SetDependentModule(pitchShiftModuleLeft);
			attenuationModuleRight->SetDependentModule(pitchShiftModuleRight);

			pitchShiftModuleLeft->SetDependentModule(loopedAudioModuleLeft);
			pitchShiftModuleRight->SetDependentModule(loopedAudioModuleRight);

			// TODO: Will this be slow?
			if (!loopedAudioModuleLeft->UseLoopedAudioData(leftAudioData, 0, error))
				return false;

			if (!loopedAudioModuleRight->UseLoopedAudioData(rightAudioData, 0, error))
				return false;

			// TODO: Enable these two lines when ready.
			//pitchShiftModuleLeft->SetSourceAndTargetFrequencies(pitchData->GetAnalyticalPitch(), noteFrequency);
			//pitchShiftModuleRight->SetSourceAndTargetFrequencies(pitchData->GetAnalyticalPitch(), noteFrequency);

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
	for (uint32_t i = 0; i < soundFontData->GetNumPitchDatas(); i++)
		soundFontData->GetPitchData(i)->CalcAnalyticalPitchAndVolume(error);

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