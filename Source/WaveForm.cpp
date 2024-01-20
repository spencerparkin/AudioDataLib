#include "WaveForm.h"
#include "ComplexVector.h"
#include "Error.h"

using namespace AudioDataLib;

//---------------------------------- WaveForm ----------------------------------

WaveForm::WaveForm()
{
	this->sampleArray = new std::vector<Sample>();
	this->interpMethod = InterpolationMethod::LINEAR;
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

bool WaveForm::ConvertFromAudioBuffer(const AudioData::Format& format, const uint8_t* audioBuffer, uint64_t audioBufferSize, uint16_t channel, Error& error)
{
	if (channel >= format.numChannels)
	{
		error.Add(FormatString("Invalid channel: %d.", channel));
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
					sample.amplitude = this->CopyIntSampleFromBuffer<int8_t>(sampleBuf);
					break;
				}
				case 16:
				{
					sample.amplitude = this->CopyIntSampleFromBuffer<int16_t>(sampleBuf);
					break;
				}
				case 32:
				{
					sample.amplitude = this->CopyIntSampleFromBuffer<int32_t>(sampleBuf);
					break;
				}
				default:
				{
					error.Add(FormatString("Bad bit-depth (%d) for integers.", format.bitsPerSample));
					break;
				}
			}
		}
		else if (format.sampleType == AudioData::Format::FLOAT)
		{
			switch (format.bitsPerSample)
			{
				case 32:
				{
					sample.amplitude = this->CopyFloatSampleFromBuffer<float>(sampleBuf);
					break;
				}
				case 64:
				{
					sample.amplitude = this->CopyFloatSampleFromBuffer<double>(sampleBuf);
					break;
				}
				default:
				{
					error.Add(FormatString("Bad bit-depth (%d) for floats.", format.bitsPerSample));
					break;
				}
			}
		}
		else
		{
			error.Add(FormatString("Unknown sample type (%d) encountered.", format.sampleType));
			return false;
		}

		this->sampleArray->push_back(sample);
		i += bytesPerFrame;
	}

	return true;
}

bool WaveForm::ConvertToAudioBuffer(const AudioData::Format& format, uint8_t* audioBuffer, uint64_t audioBufferSize, uint16_t channel, Error& error) const
{
	if (channel >= format.numChannels)
	{
		error.Add(FormatString("Invalid channel: %d", channel));
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
					this->CopyIntSampleToBuffer<int8_t>(sampleBuf, amplitude);
					break;
				}
				case 16:
				{
					this->CopyIntSampleToBuffer<int16_t>(sampleBuf, amplitude);
					break;
				}
				case 32:
				{
					this->CopyIntSampleToBuffer<int32_t>(sampleBuf, amplitude);
					break;
				}
				default:
				{
					error.Add(FormatString("Bad bit-depth (%d) for integers.", format.bitsPerSample));
					return false;
				}
			}
		}
		else if (format.sampleType == AudioData::Format::FLOAT)
		{
			switch (format.bitsPerSample)
			{
				case 32:
				{
					this->CopyFloatSampleToBuffer<float>(sampleBuf, amplitude);
					break;
				}
				case 64:
				{
					this->CopyFloatSampleToBuffer<double>(sampleBuf, amplitude);
					break;
				}
				default:
				{
					error.Add(FormatString("Bad bit-depth (%d) for floats.", format.bitsPerSample));
					return false;
				}
			}
		}
		else
		{
			error.Add(FormatString("Unknown sample type (%d) encountered.", format.sampleType));
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

	double interpolatedAmplitude = 0.0;

	switch (this->interpMethod)
	{
		case InterpolationMethod::LINEAR:
		{
			double lerpAlpha = (timeSeconds - sampleBounds.minSample->timeSeconds) / (sampleBounds.maxSample->timeSeconds - sampleBounds.minSample->timeSeconds);
			interpolatedAmplitude = sampleBounds.minSample->amplitude + lerpAlpha * (sampleBounds.maxSample->amplitude - sampleBounds.minSample->amplitude);
			break;
		}
		case InterpolationMethod::CUBIC:
		{
			// TODO: Write this.  Extend given sample bounds left/right until we have 4 samples across which we can interpolate.  Invert Vandermonde matrix.
			break;
		}
	}
	
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

bool WaveForm::CalcDominantFrequencies(std::list<double>& dominantFrequenciesList, uint32_t numFrequencies, Error& error) const
{
	if (this->sampleArray->size() == 0)
	{
		error.Add("Nothing to analyze.");
		return false;
	}

	uint32_t powerOfTwo = 1;
	while (powerOfTwo << 1 < this->sampleArray->size())
		powerOfTwo <<= 1;

	ComplexVector amplitudeVector;
	for (uint32_t i = 0; i < powerOfTwo; i++)
	{
		const Sample& sample = (*this->sampleArray)[i];
		amplitudeVector.AddComponent(ComplexNumber(sample.amplitude, 0.0));
	}

	// TODO: I think these calculations are wrong.  Maybe synthesize a sin-wave, then
	//       see if we can calculate the frequency using this code.

	ComplexVector frequencyVector;
	if (!frequencyVector.FFT(amplitudeVector, false, error))
	{
		error.Add("FFT failed!");
		return false;
	}

	std::vector<double> frequencyArray;
	for (uint32_t i = 0; i < frequencyVector.Size(); i++)
		frequencyArray.push_back(frequencyVector[i].Magnitude());

	dominantFrequenciesList.clear();
	for (double frequency : frequencyArray)
	{
		if (dominantFrequenciesList.size() < numFrequencies)
			dominantFrequenciesList.push_back(frequency);
		else
		{
			for (std::list<double>::iterator iter = dominantFrequenciesList.begin(); iter != dominantFrequenciesList.end(); iter++)
			{
				double dominantFrequency = *iter;
				if (frequency > dominantFrequency)
				{
					dominantFrequenciesList.erase(iter);
					dominantFrequenciesList.push_back(frequency);
					break;
				}
			}
		}
	}

	return true;
}