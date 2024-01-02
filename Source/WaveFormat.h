#pragma once

#include "AudioFormat.h"

namespace AudioDataLib
{
	class AUDIO_DATA_LIB_API WaveFormat : public AudioFormat
	{
	public:
		WaveFormat();
		virtual ~WaveFormat();

		virtual bool ReadFromStream(std::istream& inputStream, AudioData*& audioData, std::string& error) override;
		virtual bool WriteToStream(std::ostream& outputStream, const AudioData*& audioData, std::string& error) override;
	};
}