#pragma once

#include "MidiMsgDestination.h"
#include "ByteStream.h"
#include "AudioData.h"

namespace AudioDataLib
{
	class Error;
	class SynthModule;
	
	class AUDIO_DATA_LIB_API MidiSynth : public MidiMsgDestination
	{
	public:
		MidiSynth();
		virtual ~MidiSynth();

		virtual bool Process(Error& error) override;

		virtual SynthModule* GetRootModule(uint16_t channel) = 0;

		void SetAudioStream(std::shared_ptr<AudioStream> audioStream);
		std::shared_ptr<AudioStream> GetAudioStream() { return *this->audioStream; }

		static double MidiPitchToFrequency(uint8_t pitchValue);
		static double MidiVelocityToAmplitude(uint8_t velocityValue);

		void SetMinMaxLatency(double minLatencySeconds, double maxLatencySeconds);
		void GetMinMaxLatency(double& minLatencySeconds, double& maxLatencySeconds) const;

	protected:

		std::shared_ptr<AudioStream>* audioStream;

		double minLatencySeconds;
		double maxLatencySeconds;
	};
}