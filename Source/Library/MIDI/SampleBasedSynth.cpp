#include "SampleBasedSynth.h"
#include "SoundFontData.h"
#include "LoopedAudioModule.h"
#include "Error.h"

using namespace AudioDataLib;

SampleBasedSynth::SampleBasedSynth(bool ownsAudioStream, bool ownsSoundFontData) : MidiSynth(ownsAudioStream)
{
	this->ownsSoundFontData = ownsSoundFontData;
	this->soundFontMap = new SoundFontMap();
	this->loopedAudioModule = nullptr;
}

/*virtual*/ SampleBasedSynth::~SampleBasedSynth()
{
	this->Clear();

	delete this->soundFontMap;
	delete this->loopedAudioModule;
}

/*virtual*/ bool SampleBasedSynth::ReceiveMessage(double deltaTimeSeconds, const uint8_t* message, uint64_t messageSize, Error& error)
{
	// TODO: Write this.
	return true;
}

/*virtual*/ SynthModule* SampleBasedSynth::GetRootModule(uint16_t channel)
{
	// TODO: Sort-out channel stuff.  For now, just do mono.  Note that sound-fonts typically have samples for the left and right ear, so we should accomodate this.
	if (channel != 0)
		return nullptr;

	// TODO: This is just test code.  Replace when ready.  Here we are
	//       just seeing if we can play a sample from the sound-font
	//       in a way that is accurate without artifacts.
	if (!this->loopedAudioModule)
	{
		this->loopedAudioModule = new LoopedAudioModule();
		
		SoundFontData* soundFontData = this->GetSoundFontData(1);
		if (soundFontData)
		{
			const SoundFontData::PitchData* pitchData = soundFontData->GetPitchData(0);
			if (pitchData)
			{
				const SoundFontData::LoopedAudioData* loopedAudioData = pitchData->GetLoopedAudioData(0);
				Error error;
				this->loopedAudioModule->UseLoopedAudioData(loopedAudioData, channel, error);
			}
		}
	}

	return this->loopedAudioModule;
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