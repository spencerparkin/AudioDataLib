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
#define ADL_PI					3.1415926536

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
#include <algorithm>

/** \mainpage AudioDataLib
 * 
 * \section intro_sec Introduction & Motivation
 * 
 * This project grew out of a desire to understand audio data and audio data processing algorithms.
 * It's likely mediocre at best, and you shouldn't try to use it for any real-world applications.
 * The only production use-case I have for the library presently is a simple Android mobile app.
 * Other than that, the library is not so battle-hardened that it may be worth anything to anyone.
 * Now that I've discredited the whole library, you don't have to waste any more of your time with it.
 * 
 * All that said, I'm going to use it for any of my own personal projects, and I'm hopeful that as I
 * do, the code will get better.
 * 
 * \section caps_sec Capabilities
 * 
 * Presently, the library can be used to convert between audio formats, fire-off sound-FX (fire and forget),
 * load and play MIDI files, record and save MIDI files, as well as synthesize sound as a function of
 * sound-font data and a given stream of MIDI messages.  Sound synthesis is an area of particular interest
 * to me, and I've gone as far as trying to provide reverb support which, surprisingly, adds great
 * quality and character to the sound produced.
 * 
 */