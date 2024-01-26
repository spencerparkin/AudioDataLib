#pragma once

#include "MidiSynth.h"

namespace AudioDataLib
{
	class SoundFontData;

	class AUDIO_DATA_LIB_API SampleBasedSynth : public MidiSynth
	{
	public:
		SampleBasedSynth(bool ownsAudioStream, bool ownsSoundFontData);
		virtual ~SampleBasedSynth();

		virtual bool ReceiveMessage(double deltaTimeSeconds, const uint8_t* message, uint64_t messageSize, Error& error) override;

		void SetSoundFontData(SoundFontData* soundFontData);
		SoundFontData* GetSoundFontData();

	private:
		SoundFontData* soundFontData;
		bool ownsSoundFontData;

		// TODO: Track which instruments are assigned to which MIDI channels.
		//       Respond to program change events, if possible.  Should be a
		//       way to assign instruments to channels using the command-line.
	};
}