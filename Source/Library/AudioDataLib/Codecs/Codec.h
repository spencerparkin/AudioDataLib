#pragma once

#include "AudioDataLib/Common.h"

namespace AudioDataLib
{
	class AudioData;
	class Error;
	class ByteStream;
	class ByteSwapper;

	/**
	 * @brief This is the base class for all audio codecs -- encoders/decoders.
	 * 
	 * Supporting an audio compression format is all about adding a derivative of this class.
	 * Different file formats (derivatives of the FileFormat class) can make use of various
	 * codecs when reading or writing audio files.
	 */
	class AUDIO_DATA_LIB_API Codec
	{
	public:
		Codec();
		virtual ~Codec();

		/**
		 * Decompress the data in the given input stream into the given AudioData.
		 * 
		 * @param inputStream This is a stream from which the compressed audio is to be read.
		 * @param audioOut This is the destination for the decompressed audio.  Note that the format of the audio is sometimes an input parameter.
		 * @param error This contains an error message if false is returned.
		 * @return True is returned on success; false otherwise.
		 */
		virtual bool Decode(ByteStream& inputStream, AudioData& audioOut, Error& error) = 0;

		/**
		 * Compress the data in the given AudioData into the given output stream.
		 * 
		 * @param outputStream This is the stream into which the compressed audio is to be written.
		 * @param audioIn This is the source of the decompressed audio.
		 * @param error This contains an error message if false is returned.
		 * @param True is returned on success; false otherwise.
		 */
		virtual bool Encode(ByteStream& outputStream, const AudioData& audioIn, Error& error) = 0;
	};

	/**
	 * @brief For convenience, this code simply copies raw audio data from source to destination (there is no compression or decompression.)
	 */
	class AUDIO_DATA_LIB_API RawAudioCodec : public Codec
	{
	public:
		RawAudioCodec();
		virtual ~RawAudioCodec();

		virtual bool Decode(ByteStream& inputStream, AudioData& audioOut, Error& error) override;
		virtual bool Encode(ByteStream& outputStream, const AudioData& audioIn, Error& error) override;
	};

	/**
	 * @brief This is just like the RawAudioCodec, but the audio may (or may not) be byte-swapped.
	 */
	class AUDIO_DATA_LIB_API ByteSwappedAudioCodec : public Codec
	{
	public:
		ByteSwappedAudioCodec(ByteSwapper* byteSwapper);
		virtual ~ByteSwappedAudioCodec();

		virtual bool Decode(ByteStream& inputStream, AudioData& audioOut, Error& error) override;
		virtual bool Encode(ByteStream& outputStream, const AudioData& audioIn, Error& error) override;

	private:
		ByteSwapper* byteSwapper;
	};
}