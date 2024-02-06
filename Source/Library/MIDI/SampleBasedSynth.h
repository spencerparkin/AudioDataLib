#pragma once

#include "MidiSynth.h"

namespace AudioDataLib
{
	class MixerModule;
	class SoundFontData;

	class AUDIO_DATA_LIB_API SampleBasedSynth : public MidiSynth
	{
	public:
		SampleBasedSynth(bool ownsAudioStream, bool ownsSoundFontData);
		virtual ~SampleBasedSynth();

		virtual bool ReceiveMessage(double deltaTimeSeconds, const uint8_t* message, uint64_t messageSize, Error& error) override;
		virtual SynthModule* GetRootModule(uint16_t channel) override;
		virtual bool Process(Error& error) override;
		virtual bool Initialize(Error& error) override;

		bool SetChannelInstrument(uint16_t channel, uint16_t instrument, Error& error);
		bool GetChannelInstrument(uint16_t channel, uint16_t& instrument) const;

		bool SetSoundFontData(uint16_t instrument, SoundFontData* soundFontData, bool estimateFrequencies, Error& error);
		SoundFontData* GetSoundFontData(uint16_t instrument);

		void Clear();

	private:
		bool ownsSoundFontData;
		bool estimateFrequencies;

		// This maps channel to instrument number.
		typedef std::map<uint16_t, uint16_t> ChannelMap;
		ChannelMap* channelMap;

		// This maps instrument to sound-font data.
		typedef std::map<uint16_t, SoundFontData*> SoundFontMap;
		SoundFontMap* soundFontMap;

		struct Note
		{
			uint64_t leftModuleID;
			uint64_t rightModuleID;
		};

		typedef std::map<uint8_t, Note> NoteMap;
		NoteMap* noteMap;

		MixerModule* mixerModuleLeftEar;
		MixerModule* mixerModuleRightEar;
	};
}