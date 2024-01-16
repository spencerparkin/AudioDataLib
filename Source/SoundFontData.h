#pragma once

#include "FileData.h"
#include "ChunkParser.h"

namespace AudioDataLib
{
	class AUDIO_DATA_LIB_API SoundFontData : public FileData
	{
	public:
		SoundFontData();
		virtual ~SoundFontData();
	};
}