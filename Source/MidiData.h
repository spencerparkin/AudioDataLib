#pragma once

#include "AudioCommon.h"

namespace AudioDataLib
{
	class AUDIO_DATA_LIB_API MidiData
	{
	public:
		MidiData();
		virtual ~MidiData();

		bool Load(const std::string& filePath);
		bool Save(const std::string& filePath);

		//...
	};
}