#include "WaveTableSynth.h"
#include "SoundFontData.h"

using namespace AudioDataLib;

WaveTableSynth::WaveTableSynth(bool ownsAudioStream, bool ownsSoundFontData) : MidiSynth(ownsAudioStream)
{
	this->ownsSoundFontData = ownsSoundFontData;
	this->soundFontData = nullptr;
}

/*virtual*/ WaveTableSynth::~WaveTableSynth()
{
	this->SetSoundFontData(nullptr);
}

/*virtual*/ bool WaveTableSynth::ReceiveMessage(double deltaTimeSeconds, const uint8_t* message, uint64_t messageSize, Error& error)
{
	// TODO: Write this.
	return false;
}

/*virtual*/ bool WaveTableSynth::GenerateAudio(Error& error)
{
	// TODO: Write this.
	return false;
}

void WaveTableSynth::SetSoundFontData(SoundFontData* soundFontData)
{
	if (this->ownsSoundFontData)
		delete this->soundFontData;

	this->soundFontData = soundFontData;
}

SoundFontData* WaveTableSynth::GetSoundFontData()
{
	return this->soundFontData;
}