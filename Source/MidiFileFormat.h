#pragma once

#include "FileFormat.h"
#include "ChunkParser.h"
#include "MidiData.h"

namespace AudioDataLib
{
	class AUDIO_DATA_LIB_API MidiFileFormat : public FileFormat
	{
	public:
		MidiFileFormat();
		virtual ~MidiFileFormat();

		virtual bool ReadFromStream(ByteStream& inputStream, FileData*& fileData, std::string& error) override;
		virtual bool WriteToStream(ByteStream& outputStream, FileData* fileData, std::string& error) override;

		static bool DecodeEvent(ByteStream& inputStream, MidiData::Event*& event, std::string& error);
		static bool EncodeEvent(ByteStream& outputStream, const MidiData::Event* event, std::string& error);

		static bool DecodeVariableLengthValue(uint64_t& value, ByteStream& inputStream, std::string& error);
		static bool EncodeVariableLengthValue(uint64_t value, ByteStream& outputStream, std::string& error);
	};
}