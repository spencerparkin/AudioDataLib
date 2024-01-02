#include "AudioData.h"

using namespace AudioDataLib;

AudioData::AudioData()
{
	::memset(&this->format, 0, sizeof(Format));
	this->audioStream = nullptr;
}

AudioData::AudioData(Format format, ByteStream* audioStream)
{
	this->format = format;
	this->audioStream = audioStream;
}

/*virtual*/ AudioData::~AudioData()
{
	delete this->audioStream;
}