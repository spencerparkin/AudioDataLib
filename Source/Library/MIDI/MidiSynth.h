#pragma once

#include "Common.h"
#include "ByteStream.h"
#include "AudioData.h"

namespace AudioDataLib
{
	class Error;
	class SynthModule;

	// This is the base class for all MIDI synthesizer implimentations and
	// defines the interface that all such derivatives need to provide.
	// The user could hook this up to an actual keyboard or to an instance
	// of the MidiPlayer class.
	class AUDIO_DATA_LIB_API MidiSynth
	{
	public:
		MidiSynth(bool ownsAudioStream);
		virtual ~MidiSynth();

		virtual bool ReceiveMessage(double deltaTimeSeconds, const uint8_t* message, uint64_t messageSize, Error& error);
		virtual bool GenerateAudio(Error& error);
		virtual SynthModule* GetRootModule(uint16_t channel);
		
		void SetAudioStream(AudioStream* audioStream);
		AudioStream* GetAudioStream() { return this->audioStream; }

	protected:
		
		double MidiPitchToFrequency(uint8_t pitchValue);
		double MidiVelocityToAmplitude(uint8_t velocityValue);

		AudioStream* audioStream;
		bool ownsAudioStream;

#ifdef ADL_DEBUG_SYNTH_AUDIO_STREAM
		AudioStream* debugStream;
#endif //ADL_DEBUG_SYNTH_AUDIO_STREAM

		double minLatencySeconds;
		double maxLatencySeconds;
	};
}