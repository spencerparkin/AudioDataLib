#include "WaveTableSynth.h"
#include "SoundFontData.h"

using namespace AudioDataLib;

WaveTableSynth::WaveTableSynth(const AudioData::Format& format) : MidiSynth(format)
{
	this->soundFontData = nullptr;
}

/*virtual*/ WaveTableSynth::~WaveTableSynth()
{
}

/*virtual*/ bool WaveTableSynth::ReceiveMessage(const uint8_t* message, uint64_t messageSize, Error& error)
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
	this->soundFontData = soundFontData;
}

SoundFontData* WaveTableSynth::GetSoundFontData()
{
	return this->soundFontData;
}