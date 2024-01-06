#include "AudioData.h"

using namespace AudioDataLib;

//----------------------------- AudioData -----------------------------

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

/*static*/ AudioData* AudioData::Create()
{
	return new AudioData();
}

/*static*/ void AudioData::Destroy(AudioData* audioData)
{
	delete audioData;
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

double AudioData::GetTimeSeconds() const
{
	return this->format.BytesPerChannelToSeconds(this->format.BytesPerChannel(this->audioBufferSize));
}

//----------------------------- AudioData::Format -----------------------------

double AudioData::Format::BytesToSeconds(uint64_t numBytes) const
{
	numBytes = this->RoundDownToNearestFrameMultiple(numBytes);
	return double(numBytes) / double(this->BytesPerSecond());
}

uint64_t AudioData::Format::BytesFromSeconds(double seconds) const
{
	uint64_t numBytes = uint64_t(seconds * double(this->BytesPerSecond()));
	numBytes = this->RoundDownToNearestFrameMultiple(numBytes);
	return numBytes;
}

double AudioData::Format::BytesPerChannelToSeconds(uint64_t numBytes) const
{
	return double(numBytes) / double(this->BytesPerSecondPerChannel());
}

uint64_t AudioData::Format::BytesPerChannelFromSeconds(double seconds) const
{
	return uint64_t(seconds * double(this->BytesPerSecondPerChannel()));
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

uint64_t AudioData::Format::BytesPerChannel(uint64_t audioBufferSize) const
{
	return audioBufferSize / this->numChannels;
}

uint64_t AudioData::Format::BytesPerSample() const
{
	return this->bitsPerSample / 8;
}

uint64_t AudioData::Format::SamplesPerFrame() const
{
	return this->numChannels;
}

uint64_t AudioData::Format::SamplesPerSecond() const
{
	return this->framesPerSecond * this->SamplesPerFrame();
}

uint64_t AudioData::Format::SamplesPerSecondPerChannel() const
{
	return this->framesPerSecond;
}

uint64_t AudioData::Format::BytesPerSecond() const
{
	return this->framesPerSecond * this->BytesPerFrame();
}

uint64_t AudioData::Format::BytesPerSecondPerChannel() const
{
	return this->framesPerSecond * this->BytesPerSample();
}

bool AudioData::Format::operator==(const Format& format) const
{
	return this->bitsPerSample == format.bitsPerSample &&
		this->numChannels == format.numChannels &&
		this->framesPerSecond == format.framesPerSecond;
}

bool AudioData::Format::operator!=(const Format& format) const
{
	return !(*this == format);
}