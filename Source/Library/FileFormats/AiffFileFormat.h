#pragma once

#include "FileFormat.h"

namespace AudioDataLib
{
	class AUDIO_DATA_LIB_API AiffFileFormat : public FileFormat
	{
	public:
		AiffFileFormat();
		virtual ~AiffFileFormat();

		virtual bool ReadFromStream(ByteStream& inputStream, FileData*& fileData, Error& error) override;
		virtual bool WriteToStream(ByteStream& outputStream, const FileData* fileData, Error& error) override;
	};
}