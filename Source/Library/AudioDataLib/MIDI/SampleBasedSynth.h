#pragma once

#include "AudioDataLib/MIDI/MidiSynth.h"
#include "AudioDataLib/FileDatas/WaveTableData.h"

namespace AudioDataLib
{
	class MixerModule;
	class SoundFontData;

	/**
	 * @brief This class knows how to synthesize real-time sound as a function of MIDI messages and WaveTableData.
	 */
	class AUDIO_DATA_LIB_API SampleBasedSynth : public MidiSynth
	{
	public:
		SampleBasedSynth();
		virtual ~SampleBasedSynth();

		virtual bool ReceiveMessage(double deltaTimeSeconds, const uint8_t* message, uint64_t messageSize, Error& error) override;
		virtual SynthModule* GetRootModule(uint16_t channel) override;
		virtual bool Process(Error& error) override;
		virtual bool Initialize(Error& error) override;

		bool SetChannelInstrument(uint8_t channel, uint8_t instrument, Error& error);
		bool GetChannelInstrument(uint8_t channel, uint8_t& instrument) const;

		void SetWaveTableData(std::shared_ptr<WaveTableData>& waveTableData) { *this->waveTableData = waveTableData; }
		std::shared_ptr<WaveTableData> GetWaveTableData() { return *this->waveTableData; }

		void Clear();
		void SetReverbEnabled(bool reverbEnabled);
		bool GetReverbEnabled() { return this->reverbEnabled; }

	private:
		bool estimateFrequencies;
		bool reverbEnabled;

		// This maps channel to instrument number.
		typedef std::map<uint8_t, uint8_t> ChannelMap;
		ChannelMap* channelMap;

		std::shared_ptr<WaveTableData>* waveTableData;

		struct Note
		{
			std::shared_ptr<SynthModule> leftEarModule;
			std::shared_ptr<SynthModule> rightEarModule;
		};

		typedef std::map<uint8_t, Note> NoteMap;
		NoteMap* noteMap;

		bool GenerateModuleGraph(
				const WaveTableData::AudioSampleData* audioSample,
				double noteFrequency, std::shared_ptr<SynthModule>& synthModule,
				Error& error);

		std::shared_ptr<SynthModule>* leftEarRootModule;
		std::shared_ptr<SynthModule>* rightEarRootModule;
	};
}