#pragma once

#include "FileFormat.h"
#include "ChunkParser.h"

namespace AudioDataLib
{
	class Error;

	class AUDIO_DATA_LIB_API SoundFontFormat : public FileFormat
	{
	public:
		SoundFontFormat();
		virtual ~SoundFontFormat();

		virtual bool ReadFromStream(ByteStream& inputStream, FileData*& fileData, Error& error) override;
		virtual bool WriteToStream(ByteStream& outputStream, FileData* fileData, Error& error) override;
	};
}