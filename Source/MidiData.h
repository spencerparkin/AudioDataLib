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

		//...
	};
}