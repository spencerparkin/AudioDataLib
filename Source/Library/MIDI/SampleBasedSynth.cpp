#include "SampleBasedSynth.h"
#include "SoundFontData.h"

using namespace AudioDataLib;

SampleBasedSynth::SampleBasedSynth(bool ownsAudioStream, bool ownsSoundFontData) : MidiSynth(ownsAudioStream)
{
	this->ownsSoundFontData = ownsSoundFontData;
	this->soundFontData = nullptr;
}

/*virtual*/ SampleBasedSynth::~SampleBasedSynth()
{
	this->SetSoundFontData(nullptr);
}

/*virtual*/ bool SampleBasedSynth::ReceiveMessage(double deltaTimeSeconds, const uint8_t* message, uint64_t messageSize, Error& error)
{
	// TODO: Write this.
	return false;
}

void SampleBasedSynth::SetSoundFontData(SoundFontData* soundFontData)
{
	if (this->ownsSoundFontData)
		delete this->soundFontData;

	this->soundFontData = soundFontData;
}

SoundFontData* SampleBasedSynth::GetSoundFontData()
{
	return this->soundFontData;
}