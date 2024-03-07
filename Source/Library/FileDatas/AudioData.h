#pragma once

#include "FileData.h"

namespace AudioDataLib
{
	class Error;

	/**
	 * @brief This is a container for raw audio data in a particular format, ready to be sent to a sound card,
	 *        or of how such data would appear coming from the sound card using a microphone.
	 * 
	 * Note that this kind of data is not tied to any particular file format.  As of this writing, the
	 * WaveFileFormat and AiffFileFormat classes both produce this kind of data from file.  Indirectly,
	 * an AudioData class instances is also produced from an SF2 or DLS file using the WaveFileFormat class.
	 * The general format of the sound-data here is linear PCM, ready to be fed to the hardware (for audio playback),
	 * or what we would expect from the hardware (for audio capture/recording.)
	 * 
	 * In contrast, the WaveForm class is used as a format-independent space where math calculations and
	 * transforms on audio data can more easily be performed.  A typical use-case is that of resampling,
	 * or converting from one format to another.  Format conversion proceeds by simply converting first,
	 * a given AudioData instance, into a WaveForm class instance, and then back to an AudioData instance
	 * of the desired audio format.  AudioData can be multichanneled, but WaveForm data is always a single
	 * signal (mono.)
	 * 
	 * Note that audio data is not typically handed to a sound-card using this class directly.  Rather, an
	 * instance of the AudioStream class in concert with the AudioSink class is used to do that.
	 */
	class AUDIO_DATA_LIB_API AudioData : public FileData
	{
	public:
		/**
		 * @brief This structure contains all the information you need to decipher the data in
		 *        a given stream of raw audio data.
		 * 
		 * Methods are also provided that help you calculate various properties of the data.
		 * A note on terminology is in order here.  Traditionally, the term "samples per second"
		 * refers to the number of samples per second per channel, but here we mean it to be the
		 * number of frames per second times the number samples per frame.
		 * 
		 * What are samples, frames and channels?  You can think of raw audio as a big matrix
		 * where the rows are channels and the columns are frames.  Each frame is a moment in time,
		 * and each channel is an independent wave-form that plays in a different speaker (e.g., the
		 * left speaker or the right speaker in a stereo system.)  Each element of the matrix is an
		 * audio sample, represented as a signed or unsigned integer, or floating-point number.
		 * All samples will have the same representation and bit-depth.  As a stream of bytes, the
		 * audio data interleaves the channels.  That is, the data proceeds in frames, one after the
		 * other.  This is a convenient format for the sound-card, which consumes frames as time
		 * goes by, to reproduce the sound in audible form.
		 */
		struct AUDIO_DATA_LIB_API Format
		{
			enum SampleType
			{
				SIGNED_INTEGER,	// TODO: Add UNSIGNED_INTEGER type?
				FLOAT
			};

			uint16_t bitsPerSample;			///< This is the bit-depth of each sample.
			uint16_t numChannels;			///< This is the number of channels in the stream (1=mono; 2=stereo, etc.)
			uint32_t framesPerSecond;		///< This is the playback rate or sampling rate of the audio data, often measured in Hz.
			SampleType sampleType;			///< This is the format of each sample (e.g., 32-bit float, 16-bit signed integer, etc.)

			/**
			 * Tell the caller how many seconds of audio would play for the given number of bytes of audio data
			 * in this instance's format.  The given number of bytes is rounded down to the nearest multiple of
			 * the frame size before the calculation is performed.
			 */
			double BytesToSeconds(uint64_t numBytes) const;

			/**
			 * Tell the caller how many bytes of audio data would be needed to play the given number of seconds
			 * of audio in this instance's format.  The returned number of bytes is rounded down to the nearest
			 * multiple of the frame size before being returned.
			 */
			uint64_t BytesFromSeconds(double seconds) const;

			double BytesPerChannelToSeconds(uint64_t numBytes) const;
			uint64_t BytesPerChannelFromSeconds(double seconds) const;

			uint64_t RoundUpToNearestFrameMultiple(uint64_t numBytes) const;
			uint64_t RoundDownToNearestFrameMultiple(uint64_t numBytes) const;

			uint64_t BytesPerFrame() const;
			uint64_t SamplesPerFrame() const;
			uint64_t BytesPerChannel(uint64_t audioBufferSize) const;
			uint64_t BytesPerSample() const;
			uint64_t SamplesPerSecond() const;
			uint64_t SamplesPerSecondPerChannel() const;
			uint64_t BytesPerSecond() const;
			uint64_t BytesPerSecondPerChannel() const;

			bool operator==(const Format& format) const;
			bool operator!=(const Format& format) const;
		};

		AudioData();
		virtual ~AudioData();

		virtual void DumpInfo(FILE* fp) const override;
		virtual void DumpCSV(FILE* fp) const override;
		virtual FileData* Clone() const override;

		/**
		 * Allocate an instance of the AudioData class using the default constructor and return it.
		 * The main purpose of this is to make sure that memory is allocated or freed in the proper heap.
		 * It can also be useful when writing template functions.
		 */
		static AudioData* Create();

		/**
		 * Delete an instance of the AudioData class (or one of its derivatives.)
		 * The main purpose of this is to make sure that memory is allocated or freed in the proper heap.
		 * It can also be useful when writing template functions.
		 */
		static void Destroy(AudioData* audioData);

		Format& GetFormat() { return this->format; }
		const Format& GetFormat() const { return this->format; }
		void SetFormat(const Format& format) { this->format = format; }
		
		uint8_t* GetAudioBuffer() { return this->audioBuffer; }
		const uint8_t* GetAudioBuffer() const { return this->audioBuffer; }

		/**
		 * Get the size in bytes of this audio data.
		 */
		uint64_t GetAudioBufferSize() const { return this->audioBufferSize; }

		/**
		 * Set the size in bytes of this audio data.  Any present audio data is destroyed by this operation.
		 */
		void SetAudioBufferSize(uint64_t audioBufferSize);

		/**
		 * Return the number of samples in the audio stream across all channels.
		 */
		uint64_t GetNumSamples() const;

		/**
		 * Return the number of samples in a single channel of the audio stream.
		 */
		uint64_t GetNumSamplesPerChannel() const;

		/**
		 * This is really just the same thing as GetNumSamplesPerChannel, put another way.  (Unless my brain is failing me right now.)
		 */
		uint64_t GetNumFrames() const;

		/**
		 * Return the length of this audio data in seconds.
		 */
		double GetTimeSeconds() const;

		/**
		 * This structure contains information that can be gleaned from an FFT analysis of the audio data,
		 * or any other kind of analysis.
		 */
		struct MetaData
		{
			double pitch;		///< This is the sensed fundamental pitch (measured in Hz) of the audio sample, which is not necessarily the perceived pitch, but often is.
			double volume;		///< This is the sensed overall volume of the audio sample.  (I don't yet have units for this, nor is the result calculated in any reasonably conceived manner.)
		};

		/**
		 * Return the cached meta-data for this audio data.  See the CalcMetaData function.
		 */
		const MetaData& GetMetaData() const { return this->metaData; }

		/**
		 * Set the cached meta-data for this audio data.  See also the CalcMetaData function.
		 */
		void SetMetaData(const MetaData& metaData) const { this->metaData = metaData; }

		/**
		 * Calculate and cache meta-data (owned by this class instance) detailing information
		 * gleaned by an analysis of the audio data.  See the MetaData class.
		 * 
		 * @param error An Error class instance detailing error information if false is returned.
		 * @return True is returned on success; false otherwise.
		 */
		bool CalcMetaData(Error& error) const;

	protected:
		Format format;
		uint8_t* audioBuffer;
		uint64_t audioBufferSize;
		mutable MetaData metaData;
	};
}