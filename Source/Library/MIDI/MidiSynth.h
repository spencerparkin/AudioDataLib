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
		MidiSynth(bool ownsAudioStream);
		virtual ~MidiSynth();

		virtual bool Process(Error& error) override;

		virtual SynthModule* GetRootModule(uint16_t channel) = 0;

		void SetAudioStream(AudioStream* audioStream);
		AudioStream* GetAudioStream() { return this->audioStream; }

		static double MidiPitchToFrequency(uint8_t pitchValue);
		static double MidiVelocityToAmplitude(uint8_t velocityValue);

	protected:

		AudioStream* audioStream;
		bool ownsAudioStream;

#ifdef ADL_DEBUG_SYNTH_AUDIO_STREAM
		AudioStream* debugStream;
#endif //ADL_DEBUG_SYNTH_AUDIO_STREAM

		double minLatencySeconds;
		double maxLatencySeconds;
	};
}