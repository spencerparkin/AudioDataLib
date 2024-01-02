#pragma once

#include "AudioCommon.h"

namespace AudioDataLib
{
	class AudioData;

	class AUDIO_DATA_LIB_API AudioFormat
	{
	public:
		AudioFormat();
		virtual ~AudioFormat();

		virtual bool ReadFromStream(std::istream& inputStream, AudioData*& audioData, std::string& error) = 0;
		virtual bool WriteToStream(std::ostream& outputStream, const AudioData*& audioData, std::string& error) = 0;
	};
}