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

double AudioData::Format::BytesToSeconds(uint64_t numBytes) const
{
	uint64_t sampleRateBytesPerSecond = this->sampleRateBitsPerSecond / 8;
	double seconds = double(sampleRateBytesPerSecond * numBytes);
	return seconds;
}

uint64_t AudioData::Format::BytesFromSeconds(double seconds) const
{
	double sampleRateBytesPerSecond = double(this->sampleRateBitsPerSecond / 8);
	uint64_t numBytes = uint64_t(seconds / sampleRateBytesPerSecond);
	return numBytes;
}

uint64_t AudioData::Format::RoundUpToNearestFrameMultiple(uint64_t numBytes) const
{
	uint64_t bytesPerFrame = this->BytesPerFrame();
	uint64_t remainder = numBytes % bytesPerFrame;
	numBytes += bytesPerFrame - remainder;
	return numBytes;
}

uint64_t AudioData::Format::RoundDownToNearestFrameMultiple(uint64_t numBytes) const
{
	uint64_t bytesPerFrame = this->BytesPerFrame();
	uint64_t remainder = numBytes % bytesPerFrame;
	numBytes -= remainder;
	return numBytes;
}

uint64_t AudioData::Format::BytesPerFrame() const
{
	uint64_t bytesPerSample = this->BytesPerSample();
	uint64_t samplesPerFrame = this->numChannels;
	uint64_t bytesPerFrame = bytesPerSample * samplesPerFrame;
	return bytesPerFrame;
}

uint64_t AudioData::Format::BytesPerSample() const
{
	return this->bitsPerSample / 8;
}