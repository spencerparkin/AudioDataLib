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

	protected:
		// It is always assumed that the given buffer has at least 4 bytes available.
		void EncodeVariableLengthValue(uint64_t value, uint8_t* buffer);
		void DecodeVariableLengthValue(uint64_t& value, const uint8_t* buffer);
	};
}