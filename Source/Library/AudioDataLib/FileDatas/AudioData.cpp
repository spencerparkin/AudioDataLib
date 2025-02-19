#include "AudioDataLib/FileDatas/AudioData.h"
#include "AudioDataLib/WaveForm.h"
#include "AudioDataLib/FrequencyGraph.h"
#include "AudioDataLib/ErrorSystem.h"

using namespace AudioDataLib;

//----------------------------- AudioData -----------------------------

AudioData::AudioData()
{
	::memset(&this->format, 0, sizeof(Format));
	::memset(&this->metaData, 0, sizeof(MetaData));
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

/*virtual*/ FileData* AudioData::Clone() const
{
	auto audioData = new AudioData();
	audioData->SetAudioBufferSize(this->GetAudioBufferSize());
	::memcpy(audioData->GetAudioBuffer(), this->GetAudioBuffer(), (size_t)audioData->GetAudioBufferSize());
	audioData->SetFormat(this->GetFormat());
	return audioData;
}

/*virtual*/ void AudioData::DumpInfo(FILE* fp) const
{
	const char* sampleTypeStr = nullptr;

	switch (this->format.sampleType)
	{
	case Format::SampleType::FLOAT:
		sampleTypeStr = "IEEE floating-point";
		break;
	case Format::SampleType::SIGNED_INTEGER:
		sampleTypeStr = "Signed integer";
		break;
	case Format::SampleType::UNSIGNED_INTEGER:
		sampleTypeStr = "Unsigned integer";
		break;
	default:
		sampleTypeStr = "?";
		break;
	}

	double durationSeconds = this->GetTimeSeconds();

	this->GetMetaData();

	fprintf(fp, "Sample-type: %s\n", sampleTypeStr);
	fprintf(fp, "Bits-per-spample: %d\n", this->format.bitsPerSample);
	fprintf(fp, "Frame-rate (FPS): %d\n", this->format.framesPerSecond);
	fprintf(fp, "Channels: %d\n", this->format.numChannels);
	fprintf(fp, "Buffer size: %lld\n", this->audioBufferSize);
	fprintf(fp, "Duration (sec): %f\n", durationSeconds);
	fprintf(fp, "Pitch: %f\n", this->metaData.pitch);
	fprintf(fp, "Volume: %f\n", this->metaData.volume);
}

/*virtual*/ void AudioData::DumpCSV(FILE* fp) const
{
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

uint64_t AudioData::GetNumFrames() const
{
	return this->GetAudioBufferSize() / this->format.BytesPerFrame();
}

double AudioData::GetTimeSeconds() const
{
	return this->format.BytesPerChannelToSeconds(this->format.BytesPerChannel(this->audioBufferSize));
}

bool AudioData::CalcMetaData() const
{
	WaveForm waveForm;
	if (!waveForm.ConvertFromAudioBuffer(format, this->audioBuffer, this->audioBufferSize, 0))
		return false;

	this->metaData.volume = waveForm.CalcAverageVolume();

	// Note that if the number of samples here is too low, we can lose accuracy.
	// But the same is true if we request too many samples!  Maybe due to round-off error in the math?
	FrequencyGraph frequencyGraph;
	if (!frequencyGraph.FromWaveForm(waveForm, 32768))
		return false;

	// TODO: Being able to accurately estimate the fundamental frequency of a given wave-form is
	//       an ongoing problem.  More research is needed.  Note that it shouldn't be necessary
	//       to do this in order to be able to use an SF file, because pitch information is
	//       embedded in the file.
	this->metaData.pitch = frequencyGraph.EstimateFundamentalFrequency();

	return true;
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