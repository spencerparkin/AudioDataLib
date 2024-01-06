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

// TODO: Provide support for MIDI files.
//       See: https://github.com/colxi/midi-parser-js/wiki/MIDI-File-Format-Specifications
//       Being able to read/write MIDI files and exposing all features is not the main goal.
//       Basic saving and loading is good, but the main goal is to provide a convenient
//       mechanism that an application can use to feed MIDI file data to an opened MIDI
//       device for real-time synthesis and audio playback of the MIDI file.