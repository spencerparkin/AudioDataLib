#include "WaveForm.h"

using namespace AudioDataLib;

//---------------------------------- WaveForm ----------------------------------

WaveForm::WaveForm()
{
	this->sampleArray = new std::vector<Sample>();
}

/*virtual*/ WaveForm::~WaveForm()
{
	delete this->sampleArray;
}

void WaveForm::Clear()
{
	this->sampleArray->clear();
}

void WaveForm::MakeSilence(double samplesPerSecond, double totalSeconds)
{
	this->Clear();

	uint64_t numSamples = uint64_t(samplesPerSecond * totalSeconds);
	for (uint64_t i = 0; i < numSamples; i++)
	{
		Sample sample;
		sample.amplitude = 0.0;
		sample.timeSeconds = (double(i) / double(numSamples - 1)) * totalSeconds;
		this->sampleArray->push_back(sample);
	}
}

uint64_t WaveForm::GetSizeBytes(const AudioData::Format& format, bool allChannels) const
{
	if (this->sampleArray->size() == 0)
		return 0;

	uint64_t numBytes = format.BytesPerChannelFromSeconds(this->GetTimespan());
	numBytes += format.BytesPerSample();

	if (allChannels)
		numBytes *= format.numChannels;

	return numBytes;
}

bool WaveForm::ConvertFromAudioBuffer(const AudioData::Format& format, const uint8_t* audioBuffer, uint64_t audioBufferSize, uint16_t channel, std::string& error)
{
	if (channel >= format.numChannels)
	{
		error = "Invalid channel.";
		return false;
	}

	this->Clear();

	uint64_t bytesPerSample = format.BytesPerSample();
	uint64_t samplesPerFrame = format.SamplesPerFrame();
	uint64_t bytesPerFrame = bytesPerSample * samplesPerFrame;

	uint64_t i = 0;
	uint32_t number = 0;
	while (i < audioBufferSize)
	{
		Sample sample;
		sample.timeSeconds = format.BytesToSeconds(i);

		const uint8_t* frameBuf = &audioBuffer[i];
		const uint8_t* sampleBuf = &frameBuf[bytesPerSample * channel];

		if (format.sampleType == AudioData::Format::SIGNED_INTEGER)
		{
			switch (format.bitsPerSample)
			{
				case 8:
				{
					sample.amplitude = this->CopySampleFromBuffer<int8_t>(sampleBuf);
					break;
				}
				case 16:
				{
					sample.amplitude = this->CopySampleFromBuffer<int16_t>(sampleBuf);
					break;
				}
				case 32:
				{
					sample.amplitude = this->CopySampleFromBuffer<int32_t>(sampleBuf);
					break;
				}
			}
		}
		else if (format.sampleType == AudioData::Format::FLOAT)
		{
			assert(format.bitsPerSample == 32);
			sample.amplitude = this->CopyFloatSampleFromBuffer(sampleBuf);
		}
		else
		{
			error = "Unknown sample type encountered.";
			return false;
		}

		this->sampleArray->push_back(sample);
		i += bytesPerFrame;
	}

	return true;
}

bool WaveForm::ConvertToAudioBuffer(const AudioData::Format& format, uint8_t* audioBuffer, uint64_t audioBufferSize, uint16_t channel, std::string& error) const
{
	if (channel >= format.numChannels)
	{
		error = "Invalid channel.";
		return false;
	}

	uint64_t bytesPerSample = format.BytesPerSample();
	uint64_t samplesPerFrame = format.SamplesPerFrame();
	uint64_t bytesPerFrame = bytesPerSample * samplesPerFrame;

	uint64_t i = 0;
	uint32_t number = 0;
	while (i < audioBufferSize)
	{
		uint8_t* frameBuf = &audioBuffer[i];
		uint8_t* sampleBuf = &frameBuf[bytesPerSample * channel];

		double timeSeconds = format.BytesToSeconds(i);
		double amplitude = this->EvaluateAt(timeSeconds);
		
		if (format.sampleType == AudioData::Format::SIGNED_INTEGER)
		{
			switch (format.bitsPerSample)
			{
				case 8:
				{
					this->CopySampleToBuffer<int8_t>(sampleBuf, amplitude);
					break;
				}
				case 16:
				{
					this->CopySampleToBuffer<int16_t>(sampleBuf, amplitude);
					break;
				}
				case 32:
				{
					this->CopySampleToBuffer<int32_t>(sampleBuf, amplitude);
					break;
				}
			}
		}
		else if (format.sampleType == AudioData::Format::FLOAT)
		{
			assert(format.bitsPerSample == 32);
			this->CopyFloatSampleToBuffer(sampleBuf, amplitude);
		}
		else
		{
			error = "Unknown sample type encountered.";
			return false;
		}

		i += bytesPerFrame;
	}

	return true;
}

double WaveForm::EvaluateAt(double timeSeconds) const
{
	SampleBounds sampleBounds{ nullptr, nullptr };
	if (!this->FindTightestSampleBounds(timeSeconds, sampleBounds))
		return 0.0;

	// TODO: Do a cubic interpolation instead?  Doing so isn't that hard.  You just have to invert a Vandermonde matrix.
	double lerpAlpha = (timeSeconds - sampleBounds.minSample->timeSeconds) / (sampleBounds.maxSample->timeSeconds - sampleBounds.minSample->timeSeconds);
	double interpolatedAmplitude = sampleBounds.minSample->amplitude + lerpAlpha * (sampleBounds.maxSample->amplitude - sampleBounds.minSample->amplitude);
	return interpolatedAmplitude;
}

bool WaveForm::SampleBounds::ContainsTime(double timeSeconds) const
{
	return this->minSample->timeSeconds <= timeSeconds && timeSeconds <= this->maxSample->timeSeconds;
}

bool WaveForm::FindTightestSampleBounds(double timeSeconds, SampleBounds& sampleBounds) const
{
	if (this->sampleArray->size() == 0)
		return false;

	uint32_t i = 0;
	uint32_t j = (uint32_t)this->sampleArray->size() - 1;

	sampleBounds.minSample = &(*this->sampleArray)[i];
	sampleBounds.maxSample = &(*this->sampleArray)[j];

	if (!sampleBounds.ContainsTime(timeSeconds))
		return false;

	while (i + 1 < j)
	{
		uint32_t k = (i + j) / 2;
		assert(i < k && k < j);

		SampleBounds leftSampleBounds{ sampleBounds.minSample, &(*this->sampleArray)[k] };
		SampleBounds rightSampleBounds{ &(*this->sampleArray)[k], sampleBounds.maxSample };

		if (leftSampleBounds.ContainsTime(timeSeconds))
		{
			sampleBounds = leftSampleBounds;
			j = k;
		}
		else
		{
			sampleBounds = rightSampleBounds;
			i = k;
			assert(sampleBounds.ContainsTime(timeSeconds));
		}
	}

	return true;
}

void WaveForm::Copy(const WaveForm* waveForm)
{
	this->Clear();

	for (const Sample& sample : *waveForm->sampleArray)
		this->sampleArray->push_back(sample);
}

void WaveForm::SumTogether(const std::list<WaveForm*>& waveFormList)
{
	this->Clear();

	if (waveFormList.size() == 0)
		return;

	if (waveFormList.size() == 1)
	{
		this->Copy(*waveFormList.begin());
		return;
	}

	double minStartTime = std::numeric_limits<double>::max();
	double maxEndTime = std::numeric_limits<double>::min();
	double maxAvgSamplesPerSecond = std::numeric_limits<double>::min();
	for (const WaveForm* waveForm : waveFormList)
	{
		double avgSamplesPerSecond = waveForm->AverageSampleRate();
		if (avgSamplesPerSecond > maxAvgSamplesPerSecond)
			maxAvgSamplesPerSecond = avgSamplesPerSecond;

		double startTime = waveForm->GetStartTime();
		if (startTime < minStartTime)
			minStartTime = startTime;

		double endTime = waveForm->GetEndTime();
		if (endTime > maxEndTime)
			maxEndTime = endTime;
	}

	double timeSpanSeconds = maxEndTime - minStartTime;
	uint32_t numSamples = uint32_t(timeSpanSeconds * maxAvgSamplesPerSecond);
	for (uint32_t i = 0; i < numSamples; i++)
	{
		Sample sample;
		sample.timeSeconds = minStartTime + (double(i) / double(numSamples - 1)) * timeSpanSeconds;
		sample.amplitude = 0.0;
		for (const WaveForm* waveForm : waveFormList)
			sample.amplitude += waveForm->EvaluateAt(sample.timeSeconds);
		this->sampleArray->push_back(sample);
	}
}

void WaveForm::Clamp(double minAmplitude, double maxAmplitude)
{
	for (Sample& sample : *this->sampleArray)
	{
		if (sample.amplitude < minAmplitude)
			sample.amplitude = minAmplitude;
		if (sample.amplitude > maxAmplitude)
			sample.amplitude = maxAmplitude;
	}
}

double WaveForm::AverageSampleRate() const
{
	double timeSpanSeconds = this->GetTimespan();
	if (timeSpanSeconds == 0.0)
		return 0.0;

	double averageSamplesPerSeconds = double(this->sampleArray->size()) / timeSpanSeconds;
	return averageSamplesPerSeconds;
}

double WaveForm::GetStartTime() const
{
	if (this->sampleArray->size() == 0)
		return 0.0;

	return (*this->sampleArray)[0].timeSeconds;
}

double WaveForm::GetEndTime() const
{
	if (this->sampleArray->size() == 0)
		return 0.0;

	return (*this->sampleArray)[this->sampleArray->size() - 1].timeSeconds;
}

double WaveForm::GetTimespan() const
{
	return this->GetEndTime() - this->GetStartTime();
}

uint64_t WaveForm::GetNumSamples() const
{
	return this->sampleArray->size();
}

double WaveForm::GetMaxAmplitude() const
{
	double maxAmplitude = std::numeric_limits<double>::min();
	for (const Sample& sample : *this->sampleArray)
		if (sample.amplitude > maxAmplitude)
			maxAmplitude = sample.amplitude;

	return maxAmplitude;
}

double WaveForm::GetMinAmplitude() const
{
	double minAmplitude = std::numeric_limits<double>::max();
	for (const Sample& sample : *this->sampleArray)
		if (sample.amplitude < minAmplitude)
			minAmplitude = sample.amplitude;

	return minAmplitude;
}

bool WaveForm::Renormalize()
{
	double minAmplitude = this->GetMinAmplitude();
	double maxAmplitude = this->GetMaxAmplitude();
	double absAmplitude = ADL_MAX(ADL_ABS(minAmplitude), ADL_ABS(maxAmplitude));
	if (absAmplitude == 0.0)
		return false;

	double scale = 1.0 / absAmplitude;
	if (::isinf(scale) || ::isnan(scale))
		return false;

	this->Scale(scale);
	return true;
}

void WaveForm::Scale(double scale)
{
	for (Sample& sample : *this->sampleArray)
		sample.amplitude *= scale;
}