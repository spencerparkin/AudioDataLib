#include "AudioData.h"

using namespace AudioDataLib;

AudioData::AudioData()
{
	::memset(&this->format, 0, sizeof(Format));
	this->audioBuffer = nullptr;
	this->audioBufferSize = 0;
}

/*virtual*/ AudioData::~AudioData()
{
	this->SetAudioBufferSize(0);
}

void AudioData::SetAudioBufferSize(uint64_t audioBufferSize)
{
	delete[] this->audioBuffer;
	this->audioBuffer = nullptr;
	this->audioBufferSize = audioBufferSize;
	if (this->audioBufferSize > 0)
	{
		this->audioBuffer = new uint8_t[(uint32_t)this->audioBufferSize];
		::memset(this->audioBuffer, 0, (size_t)this->audioBufferSize);
	}
}

uint64_t AudioData::GetNumSamples() const
{
	return this->GetAudioBufferSize() / this->format.BytesPerSample();
}

uint64_t AudioData::GetNumSamplesPerChannel() const
{
	return this->GetNumSamples() / this->format.numChannels;
}

double AudioData::Format::BytesToSeconds(uint64_t numBytes) const
{
	return double(numBytes) / double(this->BytesPerSecond());
}

uint64_t AudioData::Format::BytesFromSeconds(double seconds) const
{
	return uint64_t(seconds * double(this->BytesPerSecond()));
}

uint64_t AudioData::Format::RoundUpToNearestFrameMultiple(uint64_t numBytes) const
{
	uint64_t bytesPerFrame = this->BytesPerFrame();
	uint64_t remainder = numBytes % bytesPerFrame;
	if (remainder > 0)
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
	return this->BytesPerSample() * this->SamplesPerFrame();
}

uint64_t AudioData::Format::BytesPerSample() const
{
	return this->bitsPerSample / 8;
}

uint64_t AudioData::Format::SamplesPerFrame() const
{
	return this->numChannels;
}

uint64_t AudioData::Format::FramesPerSecond() const
{
	return this->samplesPerSecond / this->SamplesPerFrame();
}

uint64_t AudioData::Format::BytesPerSecond() const
{
	return this->samplesPerSecond * this->BytesPerSample();
}

bool AudioData::Format::operator==(const Format& format) const
{
	return this->bitsPerSample == format.bitsPerSample &&
		this->numChannels == format.numChannels &&
		this->samplesPerSecond == format.samplesPerSecond;
}

bool AudioData::Format::operator!=(const Format& format) const
{
	return !(*this == format);
}