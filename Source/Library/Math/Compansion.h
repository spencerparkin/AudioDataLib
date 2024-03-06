#pragma once

#include "Common.h"

namespace AudioDataLib
{
	class AUDIO_DATA_LIB_API Compansion
	{
	public:
		Compansion();
		virtual ~Compansion();

		uint8_t ULawCompress(int16_t sample);
		int16_t ULawExpand(uint8_t sample);

		uint8_t ALawCompress(int16_t sample);
		int16_t ALawExpand(uint8_t sample);

	private:
		struct ULawTableEntry
		{
			uint8_t rangeCode;
			int16_t minSample, maxSample;
			int16_t intervalSize;
		};

		std::vector<ULawTableEntry>* ulawTableArray;
	};
}