#pragma once

#include "FileFormat.h"
#include "ChunkParser.h"
#include "MidiData.h"

namespace AudioDataLib
{
	class Error;

	/**
	 * @brief This class can read and write MIDI files.
	 * 
	 * For now, not all MIDI file features are supported here, but many are.  In any case, it's a good start.
	 */
	class AUDIO_DATA_LIB_API MidiFileFormat : public FileFormat
	{
	public:
		MidiFileFormat();
		virtual ~MidiFileFormat();

		virtual bool ReadFromStream(ByteStream& inputStream, FileData*& fileData, Error& error) override;
		virtual bool WriteToStream(ByteStream& outputStream, const FileData* fileData, Error& error) override;

		static bool DecodeEvent(ByteStream& inputStream, MidiData::Event*& event, Error& error);
		static bool EncodeEvent(ByteStream& outputStream, const MidiData::Event* event, Error& error);

		static bool DecodeVariableLengthValue(uint64_t& value, ByteStream& inputStream, Error& error);
		static bool EncodeVariableLengthValue(uint64_t value, ByteStream& outputStream, Error& error);
	};
}