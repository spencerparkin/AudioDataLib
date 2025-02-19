#pragma once

#include "AudioDataLib/FileFormats/FileFormat.h"
#include "AudioDataLib/ChunkParser.h"
#include "AudioDataLib/FileDatas/MidiData.h"

namespace AudioDataLib
{
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

		virtual bool ReadFromStream(ByteStream& inputStream, std::unique_ptr<FileData>& fileData) override;
		virtual bool WriteToStream(ByteStream& outputStream, const FileData* fileData) override;

		static bool DecodeEvent(ByteStream& inputStream, MidiData::Event*& event);
		static bool EncodeEvent(ByteStream& outputStream, const MidiData::Event* event);

		static bool DecodeVariableLengthValue(uint64_t& value, ByteStream& inputStream);
		static bool EncodeVariableLengthValue(uint64_t value, ByteStream& outputStream);
	};
}