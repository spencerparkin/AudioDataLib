#include "AudioDataLib/FileFormats/FileFormat.h"
#include "AudioDataLib/FileFormats/WaveFileFormat.h"
#include "AudioDataLib/FileFormats/MidiFileFormat.h"
#include "AudioDataLib/FileFormats/SoundFontFormat.h"
#include "AudioDataLib/FileFormats/AiffFileFormat.h"
#include "AudioDataLib/FileFormats/DownloadableSoundFormat.h"

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

	if (filePath.find(".aiff") != std::string::npos || filePath.find(".AIFF") != std::string::npos ||
		filePath.find(".aif") != std::string::npos || filePath.find(".AIF") != std::string::npos)
	{
		fileFormat.reset(new AiffFileFormat());
	}

	if (filePath.find(".mid") != std::string::npos || filePath.find(".MID") != std::string::npos)
		fileFormat.reset(new MidiFileFormat());

	if (filePath.find(".sf2") != std::string::npos || filePath.find(".SF2") != std::string::npos)
		fileFormat.reset(new SoundFontFormat());

	if (filePath.find(".dls") != std::string::npos || filePath.find(".DLS") != std::string::npos)
		fileFormat.reset(new DownloadableSoundFormat());

	return fileFormat;
}