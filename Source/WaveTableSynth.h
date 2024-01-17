#pragma once

#include "MidiSynth.h"

namespace AudioDataLib
{
	class SoundFontData;

	class AUDIO_DATA_LIB_API WaveTableSynth : public MidiSynth
	{
	public:
		WaveTableSynth(const AudioData::Format& format);
		virtual ~WaveTableSynth();

		virtual bool ReceiveMessage(const uint8_t* message, uint64_t messageSize, Error& error) override;
		virtual bool GenerateAudio(Error& error) override;

		void SetSoundFontData(SoundFontData* soundFontData);
		SoundFontData* GetSoundFontData();

	private:
		SoundFontData* soundFontData;
	};
}