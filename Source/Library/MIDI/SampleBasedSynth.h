#pragma once

#include "MidiSynth.h"
#include "SoundFontData.h"

namespace AudioDataLib
{
	class MixerModule;
	class SoundFontData;

	class AUDIO_DATA_LIB_API SampleBasedSynth : public MidiSynth
	{
	public:
		SampleBasedSynth();
		virtual ~SampleBasedSynth();

		virtual bool ReceiveMessage(double deltaTimeSeconds, const uint8_t* message, uint64_t messageSize, Error& error) override;
		virtual SynthModule* GetRootModule(uint16_t channel) override;
		virtual bool Process(Error& error) override;
		virtual bool Initialize(Error& error) override;

		bool SetChannelInstrument(uint16_t channel, uint16_t instrument, Error& error);
		bool GetChannelInstrument(uint16_t channel, uint16_t& instrument) const;

		bool SetSoundFontData(uint16_t instrument, std::shared_ptr<SoundFontData>& soundFontData, bool estimateFrequencies, Error& error);
		SoundFontData* GetSoundFontData(uint16_t instrument);

		void Clear();
		void SetReverbEnabled(bool reverbEnabled);
		bool GetReverbEnabled() { return this->reverbEnabled; }

	private:
		bool estimateFrequencies;
		bool reverbEnabled;

		// This maps channel to instrument number.
		typedef std::map<uint16_t, uint16_t> ChannelMap;
		ChannelMap* channelMap;

		// This maps instrument to sound-font data.
		typedef std::map<uint16_t, std::shared_ptr<SoundFontData>> SoundFontMap;
		SoundFontMap* soundFontMap;

		struct Note
		{
			std::shared_ptr<SynthModule> leftEarModule;
			std::shared_ptr<SynthModule> rightEarModule;
		};

		typedef std::map<uint8_t, Note> NoteMap;
		NoteMap* noteMap;

		bool GenerateModuleGraph(
				const SoundFontData::AudioSample* audioSample,
				SoundFontData::LoopedAudioData::ChannelType channelType,
				double noteFrequency, std::shared_ptr<SynthModule>& synthModule,
				Error& error);

		std::shared_ptr<SynthModule>* leftEarRootModule;
		std::shared_ptr<SynthModule>* rightEarRootModule;
	};
}