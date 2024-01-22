#pragma once

#include "Common.h"

namespace AudioDataLib
{
	AUDIO_DATA_LIB_API std::string FormatString(const char* format, ...);

	// I really don't like throwing exceptions or trying to catch them either.
	// My error handling stratagy may be out-dated in the eyes of some, but
	// I believe it results in better software.
	class AUDIO_DATA_LIB_API Error
	{
	public:
		Error();
		virtual ~Error();

		void Add(const std::string& error);
		std::string GetErrorMessage() const;
		operator bool() const;
		void Clear();

	protected:
		std::vector<std::string>* errorArray;
	};
}