#pragma once

#include "FileData.h"

namespace AudioDataLib
{
	class AUDIO_DATA_LIB_API MidiData : public FileData
	{
	public:
		MidiData();
		virtual ~MidiData();

		static MidiData* Create();
		static void Destroy(MidiData* midiData);

		enum FormatType
		{
			SINGLE_TRACK = 0,
			MULTI_TRACK = 1,
			VARIOUS_TRACKS = 2
		};

		struct Timing
		{
			union
			{
				uint32_t ticksPerBeat;
				struct
				{
					uint16_t framesPerSecond;
					uint16_t ticksPerFrame;
				};
			};

			enum Type
			{
				TICKS_PER_BEAT,
				FRAMES_PER_SECOND
			};

			Type type;
		};

		FormatType formatType;
		Timing timing;
	};
}