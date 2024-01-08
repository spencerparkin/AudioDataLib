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
	if (filePath.find(".wav", 0) > 0 || filePath.find(".WAV", 0) > 0)
		return new WaveFileFormat();

	if (filePath.find(".midi", 0) > 0 || filePath.find(".MIDI", 0) > 0)
		return new MidiFileFormat();

	return nullptr;
}

/*static*/ void FileFormat::Destroy(FileFormat* fileFormat)
{
	delete fileFormat;
}