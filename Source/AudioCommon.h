#pragma once

#if defined AUDIO_DATA_LIB_EXPORT
#	define AUDIO_DATA_LIB_API		__declspec(dllexport)
#elif defined AUDIO_DATA_LIB_IMPORT
#	define AUDIO_DATA_LIB_API		__declspec(dllimport)
#else
#	define AUDIO_DATA_LIB_API
#endif

#include <string>
#include <vector>
#include <list>