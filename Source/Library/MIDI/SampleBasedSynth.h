#pragma once

#include "MidiSynth.h"

namespace AudioDataLib
{
	class SoundFontData;
	class LoopedAudioModule;

	class AUDIO_DATA_LIB_API SampleBasedSynth : public MidiSynth
	{
	public:
		SampleBasedSynth(bool ownsAudioStream, bool ownsSoundFontData);
		virtual ~SampleBasedSynth();

		virtual bool ReceiveMessage(double deltaTimeSeconds, const uint8_t* message, uint64_t messageSize, Error& error) override;
		virtual SynthModule* GetRootModule(uint16_t channel) override;

		void SetSoundFontData(uint16_t channel, SoundFontData* soundFontData);
		SoundFontData* GetSoundFontData(uint16_t channel);

		void Clear();

	private:
		bool ownsSoundFontData;

		typedef std::map<uint16_t, SoundFontData*> SoundFontMap;
		SoundFontMap* soundFontMap;

		// TODO: This is just temporary.  Replace this with a mixer module when ready.
		LoopedAudioModule* loopedAudioModule;
	};
}