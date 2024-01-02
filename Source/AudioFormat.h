#pragma once

#include "AudioCommon.h"

namespace AudioDataLib
{
	class AudioData;
	class ByteStream;

	class AUDIO_DATA_LIB_API AudioFormat
	{
	public:
		AudioFormat();
		virtual ~AudioFormat();

		virtual bool ReadFromStream(ByteStream& inputStream, AudioData*& audioData, std::string& error) = 0;
		virtual bool WriteToStream(ByteStream& outputStream, AudioData* audioData, std::string& error) = 0;
	};
}