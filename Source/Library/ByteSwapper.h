#pragma once

#include "Common.h"

namespace AudioDataLib
{
	class AUDIO_DATA_LIB_API ByteSwapper
	{
	public:
		ByteSwapper();
		virtual ~ByteSwapper();

		uint16_t Resolve(uint16_t data) const;
		uint32_t Resolve(uint32_t data) const;
		uint64_t Resolve(uint64_t data) const;

		int16_t Resolve(int16_t data) const;
		int32_t Resolve(int32_t data) const;
		int64_t Resolve(int64_t data) const;

		bool swapsNeeded;
	};
}