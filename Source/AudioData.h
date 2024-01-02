#pragma once

#include "AudioCommon.h"
#include "ByteStream.h"

namespace AudioDataLib
{
	class AUDIO_DATA_LIB_API AudioData
	{
	public:
		AudioData();
		virtual ~AudioData();

		AudioData* Clone() const;

		struct Format
		{
			int numFrames;
			int numChannels;
			int sampleSize;
			int sampleRate;
		};

		Format format;
		MemoryStream audioStream;
	};
}