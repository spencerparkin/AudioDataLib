#pragma once

#if defined AUDIO_DATA_LIB_EXPORT
#	define AUDIO_DATA_LIB_API		__declspec(dllexport)
#elif defined AUDIO_DATA_LIB_IMPORT
#	define AUDIO_DATA_LIB_API		__declspec(dllimport)
#else
#	define AUDIO_DATA_LIB_API
#endif

#define ADL_SIGN(x)				((x) < 0.0 ? -1.0 : 1.0)
#define ADL_ABS(x)				((x) < 0.0 ? -(x) : (x))
#define ADL_MAX(x, y)			((x) > (y) ? (x) : (y))
#define ADL_MIN(x, y)			((x) < (y) ? (x) : (y))
#define ADL_CLAMP(x, a, b)		ADL_MIN(ADL_MAX(x, a), b)

#include <stdlib.h>
#include <string>
#include <vector>
#include <list>
#include <assert.h>
#include <stdint.h>
#include <memory>
#include <functional>
#include <set>
#include <time.h>
#include <stdarg.h>
#include <chrono>
#include <map>