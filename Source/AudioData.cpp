#include "AudioData.h"
#include "ByteStream.h"

using namespace AudioDataLib;

AudioData::AudioData()
{
	::memset(&this->format, 0, sizeof(Format));
	this->audioStream = new MemoryStream();
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