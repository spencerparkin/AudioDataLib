#pragma once

#include "Common.h"

namespace AudioDataLib
{
	class AUDIO_DATA_LIB_API FileData
	{
	public:
		FileData();
		virtual ~FileData();

		virtual void DumpInfo(FILE* fp) const = 0;
	};
}