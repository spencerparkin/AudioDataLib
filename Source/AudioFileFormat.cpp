#include "AudioFileFormat.h"
#include "WaveFileFormat.h"

using namespace AudioDataLib;

AudioFileFormat::AudioFileFormat()
{
}

/*virtual*/ AudioFileFormat::~AudioFileFormat()
{
}

/*static*/ AudioFileFormat* AudioFileFormat::CreateForFile(const std::string& filePath)
{
	if (filePath.find(".wav", 0) > 0 || filePath.find(".WAV", 0) > 0)
		return new WaveFileFormat();

	return nullptr;
}

/*static*/ void AudioFileFormat::Destroy(AudioFileFormat* audioFileFormat)
{
	delete audioFileFormat;
}