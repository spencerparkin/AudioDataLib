#pragma once

#include "AudioDataLib/MIDI/MidiMsgDestination.h"
#include "AudioDataLib/ByteStream.h"
#include "AudioDataLib/FileDatas/AudioData.h"

namespace AudioDataLib
{
	class SynthModule;
	
	/**
	 * @brief Derivatives of this class know how to synthesize MIDI messages into real-time audio.
	 */
	class AUDIO_DATA_LIB_API MidiSynth : public MidiMsgDestination
	{
	public:
		MidiSynth();
		virtual ~MidiSynth();

		/**
		 * This is where we strike a balance between keeping enough audio data buffered
		 * in our audio stream to prevent an audio drop-out while also not buffering too
		 * much so as to maintain the lowest possible latency.  If we don't feed the audio
		 * buffer quick enough, we periodically have to feed silence to the audio stream.
		 * If we feed the audio buffer too much, then changes to the audio (such as a note
		 * turnning on or off) don't happen soon enough, or as close to when they actually
		 * happend as we would like.  As far as drop-outs are concerned, this class is really
		 * at the mercy of the SynthModule implimentations, which needs to be as quick as
		 * they possibly can be.
		 * 
		 * A derived class might override this to do its own processing, but should call this
		 * base-class method as well.
		 */
		virtual bool Process() override;

		/**
		 * A derived class must impliment this method to provide a SynthModule that can
		 * feed the given channel.  Note that the term "channel" is overloaded.  It can
		 * refer to a channel in a stream of audio (e.g., left, right, mono, etc.), as is
		 * the case here, or it can mean a MIDI channel, of which there are 16, so don't
		 * get them confused.  Context usually makes it clear.
		 */
		virtual SynthModule* GetRootModule(uint16_t channel) = 0;

		/**
		 * The goal of this class is to feed the given AudioStream class.
		 * Typically this is chosen as a ThreadSafeAudioStream, because, while it is
		 * fed on the main thread, it is usually consumed in a time-sensative audio callback
		 * running on a dedicated audio thread.
		 */
		void SetAudioStream(std::shared_ptr<AudioStream> audioStream);

		/**
		 * Return a shared pointer to the AudioStream we're trying to continuously feed.
		 */
		std::shared_ptr<AudioStream> GetAudioStream() { return *this->audioStream; }

		/**
		 * Convert the given MIDI pitch key into a frequency.
		 * 
		 * @param pitchValue This is the MIDI key ranging from 0 to 127.
		 * @return The frequency of the note is returned; e.g., 440 Hz is an A note.
		 */
		static double MidiPitchToFrequency(uint8_t pitchValue);

		/**
		 * Convert the given MIDI velocity to an amplitude.
		 * 
		 * @param velocityValue This is the MIDI velocity ranging from 0 to 127.
		 * @return The gain is returned as a linear normalization of the given value.  I'm not sure if this is the right thing to do yet.
		 */
		static double MidiVelocityToAmplitude(uint8_t velocityValue);

		/**
		 * Adjust the given pitch frequency by the given tunning parameter.
		 * 
		 * @param pitchHz This is the pitch to adjust measured in Hz.
		 * @param fineTuneCents This is the tunning parameter measured in cents or hundredths of a semitone (half-step.)
		 * @return The adjusted pitch is returned in Hz.
		 */
		static double TunePitch(double pitchHz, int16_t fineTuneCents);

		/**
		 * Set the latency range.  The synthesizer tries to keep the amount of audio (measured in seconds)
		 * to always be somewhere in this range.  You typically want the range to be to be as small as is
		 * reasonably possible, both in size and in distance to zero.
		 */
		void SetMinMaxLatency(double minLatencySeconds, double maxLatencySeconds);

		/**
		 * Get the latency range.
		 */
		void GetMinMaxLatency(double& minLatencySeconds, double& maxLatencySeconds) const;

	protected:

		std::shared_ptr<AudioStream>* audioStream;

		double minLatencySeconds;		///< This is the minimum amount of audio (measured in seconds) that should always be buffered at any given time.
		double maxLatencySeconds;		///< This is the maximum amount of audio (measured in seconds) that should always be buffered at any given time.
	};
}