#include "FileFormat.h"
#include "WaveFileFormat.h"
#include "MidiFileFormat.h"

using namespace AudioDataLib;

FileFormat::FileFormat()
{
}

/*virtual*/ FileFormat::~FileFormat()
{
}

/*static*/ FileFormat* FileFormat::CreateForFile(const std::string& filePath)
{
	if (filePath.find(".wav") != std::string::npos || filePath.find(".WAV") != std::string::npos)
		return new WaveFileFormat();

	if (filePath.find(".mid") != std::string::npos || filePath.find(".MID") != std::string::npos)
		return new MidiFileFormat();

	return nullptr;
}

/*static*/ void FileFormat::Destroy(FileFormat* fileFormat)
{
	delete fileFormat;
}