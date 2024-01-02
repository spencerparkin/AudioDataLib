#pragma once

#include "AudioFormat.h"

namespace AudioDataLib
{
	class AUDIO_DATA_LIB_API WaveFormat : public AudioFormat
	{
	public:
		WaveFormat();
		virtual ~WaveFormat();

		virtual bool ReadFromStream(ByteStream& inputStream, AudioData*& audioData, std::string& error) override;
		virtual bool WriteToStream(ByteStream& outputStream, AudioData* audioData, std::string& error) override;
	};
}