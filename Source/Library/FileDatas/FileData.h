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
		virtual void DumpCSV(FILE* fp) const = 0;
		virtual FileData* Clone() const = 0;
	};
}