#pragma once

#include "AudioCommon.h"
#include "ByteStream.h"

namespace AudioDataLib
{
	class AUDIO_DATA_LIB_API AudioData
	{
	public:
		struct Format
		{
			int numFrames;
			int numChannels;
			int sampleSize;
			int sampleRate;
		};

		AudioData();
		AudioData(Format format, ByteStream* audioStream);
		virtual ~AudioData();

		Format format;
		ByteStream* audioStream;
	};
}