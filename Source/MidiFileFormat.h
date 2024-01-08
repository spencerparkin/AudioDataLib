#pragma once

#include "FileFormat.h"
#include "ChunkParser.h"

namespace AudioDataLib
{
	class AUDIO_DATA_LIB_API MidiFileFormat : public FileFormat
	{
	public:
		MidiFileFormat();
		virtual ~MidiFileFormat();

		virtual bool ReadFromStream(ByteStream& inputStream, FileData*& fileData, std::string& error) override;
		virtual bool WriteToStream(ByteStream& outputStream, FileData* fileData, std::string& error) override;
	};
}