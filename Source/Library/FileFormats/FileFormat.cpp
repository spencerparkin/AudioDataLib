#include "FileFormat.h"
#include "WaveFileFormat.h"
#include "MidiFileFormat.h"
#include "SoundFontFormat.h"

using namespace AudioDataLib;

FileFormat::FileFormat()
{
}

/*virtual*/ FileFormat::~FileFormat()
{
}

/*static*/ std::shared_ptr<FileFormat> FileFormat::CreateForFile(const std::string& filePath)
{
	std::shared_ptr<FileFormat> fileFormat;

	if (filePath.find(".wav") != std::string::npos || filePath.find(".WAV") != std::string::npos)
		fileFormat.reset(new WaveFileFormat());

	if (filePath.find(".mid") != std::string::npos || filePath.find(".MID") != std::string::npos)
		fileFormat.reset(new MidiFileFormat());

	if (filePath.find(".sf2") != std::string::npos || filePath.find(".SF2") != std::string::npos)
		fileFormat.reset(new SoundFontFormat());

	return fileFormat;
}