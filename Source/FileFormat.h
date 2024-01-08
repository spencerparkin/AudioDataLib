#pragma once

#include "Common.h"

namespace AudioDataLib
{
	class FileData;
	class ByteStream;

	class AUDIO_DATA_LIB_API FileFormat
	{
	public:
		FileFormat();
		virtual ~FileFormat();

		virtual bool ReadFromStream(ByteStream& inputStream, FileData*& fileData, std::string& error) = 0;
		virtual bool WriteToStream(ByteStream& outputStream, FileData* fileData, std::string& error) = 0;

		static FileFormat* CreateForFile(const std::string& filePath);
		static void Destroy(FileFormat* fileFormat);
	};
}