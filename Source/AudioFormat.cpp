#include "AudioFormat.h"
#include "WaveFormat.h"

using namespace AudioDataLib;

AudioFormat::AudioFormat()
{
}

/*virtual*/ AudioFormat::~AudioFormat()
{
}

/*static*/ AudioFormat* AudioFormat::CreateForFile(const std::string& filePath)
{
	if (filePath.find(".wav", 0) > 0 || filePath.find(".WAV", 0) > 0)
		return new WaveFormat();

	return nullptr;
}

/*static*/ void AudioFormat::Destroy(AudioFormat* audioFormat)
{
	delete audioFormat;
}