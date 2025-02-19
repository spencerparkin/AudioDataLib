#include "AudioDataLib/WaveForm.h"
#include "AudioDataLib/Math/ComplexNumber.h"
#include "AudioDataLib/ErrorSystem.h"

using namespace AudioDataLib;

//---------------------------------- WaveForm ----------------------------------

WaveForm::WaveForm()
{
	this->interpMethod = InterpolationMethod::LINEAR;
}

/*virtual*/ WaveForm::~WaveForm()
{
}

void WaveForm::Clear()
{
	this->sampleArray.clear();
}

void WaveForm::AddSample(const Sample& sample)
{
	this->sampleArray.push_back(sample);
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
		this->sampleArray.push_back(sample);
	}
}

uint64_t WaveForm::GetSizeBytes(const AudioData::Format& format, bool allChannels) const
{
	if (this->sampleArray.size() == 0)
		return 0;

	uint64_t numBytes = format.BytesPerChannelFromSeconds(this->GetTimespan());
	numBytes += format.BytesPerSample();

	if (allChannels)
		numBytes *= format.numChannels;

	return numBytes;
}

bool WaveForm::ConvertFromAudioBuffer(const AudioData::Format& format, const uint8_t* audioBuffer, uint64_t audioBufferSize, uint16_t channel)
{
	if (channel >= format.numChannels)
	{
		ErrorSystem::Get()->Add(std::format("Invalid channel: {}.", channel));
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
					ErrorSystem::Get()->Add(std::format("Bad bit-depth ({}) for signed integers.", format.bitsPerSample));
					break;
				}
			}
		}
		else if (format.sampleType == AudioData::Format::UNSIGNED_INTEGER)
		{
			switch (format.bitsPerSample)
			{
				case 8:
				{
					sample.amplitude = this->CopyUIntSampleFromBuffer<uint8_t>(sampleBuf);
					break;
				}
				case 16:
				{
					sample.amplitude = this->CopyUIntSampleFromBuffer<uint16_t>(sampleBuf);
					break;
				}
				case 32:
				{
					sample.amplitude = this->CopyUIntSampleFromBuffer<uint32_t>(sampleBuf);
					break;
				}
				default:
				{
					ErrorSystem::Get()->Add(std::format("Bad bit-depth ({}) for unsigned integers.", format.bitsPerSample));
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
					ErrorSystem::Get()->Add(std::format("Bad bit-depth ({}) for floats.", format.bitsPerSample));
					break;
				}
			}
		}
		else
		{
			ErrorSystem::Get()->Add(std::format("Unknown sample type ({}) encountered.", int(format.sampleType)));
			return false;
		}

		this->sampleArray.push_back(sample);
		i += bytesPerFrame;
	}

	return true;
}

bool WaveForm::ConvertToAudioBuffer(const AudioData::Format& format, uint8_t* audioBuffer, uint64_t audioBufferSize, uint16_t channel) const
{
	if (channel >= format.numChannels)
	{
		ErrorSystem::Get()->Add(std::format("Invalid channel: {}", channel));
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
					ErrorSystem::Get()->Add(std::format("Bad bit-depth ({}) for signed integers.", format.bitsPerSample));
					return false;
				}
			}
		}
		else if (format.sampleType == AudioData::Format::UNSIGNED_INTEGER)
		{
			switch (format.bitsPerSample)
			{
				case 8:
				{
					this->CopyUIntSampleToBuffer<uint8_t>(sampleBuf, amplitude);
					break;
				}
				case 16:
				{
					this->CopyUIntSampleToBuffer<uint16_t>(sampleBuf, amplitude);
					break;
				}
				case 32:
				{
					this->CopyUIntSampleToBuffer<uint32_t>(sampleBuf, amplitude);
					break;
				}
				default:
				{
					ErrorSystem::Get()->Add(std::format("Bad bit-depth ({}) for unsigned integers.", format.bitsPerSample));
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
					ErrorSystem::Get()->Add(std::format("Bad bit-depth ({}) for floats.", format.bitsPerSample));
					return false;
				}
			}
		}
		else
		{
			ErrorSystem::Get()->Add(std::format("Unknown sample type ({}) encountered.", int(format.sampleType)));
			return false;
		}

		i += bytesPerFrame;
	}

	return true;
}

void WaveForm::Interpolate(const SampleBounds& sampleBounds, double timeSeconds, Sample& interpolatedSample) const
{
	interpolatedSample.timeSeconds = timeSeconds;

	if (sampleBounds.minSample == sampleBounds.maxSample)
	{
		interpolatedSample = *sampleBounds.minSample;
		return;
	}

	switch (this->interpMethod)
	{
		case InterpolationMethod::LINEAR:
		{
			double lerpAlpha = (timeSeconds - sampleBounds.minSample->timeSeconds) / (sampleBounds.maxSample->timeSeconds - sampleBounds.minSample->timeSeconds);
			interpolatedSample.amplitude = sampleBounds.minSample->amplitude + lerpAlpha * (sampleBounds.maxSample->amplitude - sampleBounds.minSample->amplitude);
			break;
		}
		case InterpolationMethod::CUBIC:
		{
			uint64_t i = sampleBounds.minSample - this->sampleArray.data();
			uint64_t j = sampleBounds.maxSample - this->sampleArray.data();

			// TODO: Write this.  Extend given sample bounds left/right until we have 4 samples across which we can interpolate.  Invert Vandermonde matrix.
			break;
		}
	}
}

/*virtual*/ double WaveForm::EvaluateAt(double timeSeconds) const
{
	SampleBounds sampleBounds{ nullptr, nullptr };
	if (!this->FindTightestSampleBounds(timeSeconds, sampleBounds))
		return 0.0;

	Sample interpolatedSample;
	this->Interpolate(sampleBounds, timeSeconds, interpolatedSample);
	
	return interpolatedSample.amplitude;
}

bool WaveForm::SampleBounds::ContainsTime(double timeSeconds) const
{
	return this->minSample->timeSeconds <= timeSeconds && timeSeconds <= this->maxSample->timeSeconds;
}

bool WaveForm::FindTightestSampleBounds(double timeSeconds, SampleBounds& sampleBounds) const
{
	if (this->sampleArray.size() == 0)
		return false;

	uint32_t i = 0;
	uint32_t j = (uint32_t)this->sampleArray.size() - 1;

	sampleBounds.minSample = &this->sampleArray[i];
	sampleBounds.maxSample = &this->sampleArray[j];

	if (!sampleBounds.ContainsTime(timeSeconds))
		return false;

	while (i + 1 < j)
	{
		uint32_t k = (i + j) / 2;
		assert(i < k && k < j);

		SampleBounds leftSampleBounds{ sampleBounds.minSample, &this->sampleArray[k] };
		SampleBounds rightSampleBounds{ &this->sampleArray[k], sampleBounds.maxSample };

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

	for (const Sample& sample : waveForm->sampleArray)
		this->sampleArray.push_back(sample);
}

uint64_t WaveForm::PadWithSilence(double desiredDurationSeconds, double sampleRate)
{
	uint64_t numSamplesAdded = 0;

	if (this->GetTimespan() < desiredDurationSeconds)
	{
		while (this->GetTimespan() < desiredDurationSeconds)
		{
			Sample sample;
			sample.amplitude = 0.0;

			if (this->sampleArray.size() == 0)
				sample.timeSeconds = 0.0;
			else
				sample.timeSeconds = this->sampleArray[this->sampleArray.size() - 1].timeSeconds + 1.0 / sampleRate;

			this->sampleArray.push_back(sample);
			numSamplesAdded++;
		}

		if (this->GetTimespan() > desiredDurationSeconds && this->sampleArray.size() > 0)
		{
			Sample& sample = this->sampleArray[this->sampleArray.size() - 1];
			sample.timeSeconds -= this->GetTimespan() - desiredDurationSeconds;
		}
	}

	return numSamplesAdded;
}

bool WaveForm::Trim(double startTimeSeconds, double stopTimeSeconds, bool rebaseTime)
{
	if (this->sampleArray.size() == 0)
	{
		ErrorSystem::Get()->Add("Nothing to trim.");
		return false;
	}

	if (startTimeSeconds > stopTimeSeconds)
	{
		ErrorSystem::Get()->Add(std::format("Given time bounds ([{}, {}]) doesn't make sense.", startTimeSeconds, stopTimeSeconds));
		return false;
	}

	std::vector<Sample> newSampleArray;
	for (const Sample& sample : this->sampleArray)
		if (startTimeSeconds <= sample.timeSeconds && sample.timeSeconds <= stopTimeSeconds)
			newSampleArray.push_back(sample);

	this->sampleArray = newSampleArray;

	if (rebaseTime && this->sampleArray.size() > 0)
	{
		double deltaTime = -this->sampleArray[0].timeSeconds;
		for (Sample& sample : this->sampleArray)
			sample.timeSeconds += deltaTime;
	}

	return true;
}

void WaveForm::QuickTrim(double timeSeconds, TrimSection trimSection)
{
	if (this->sampleArray.size() == 0)
		return;

	SampleBounds sampleBounds;
	if (!this->FindTightestSampleBounds(timeSeconds, sampleBounds))
	{
		if ((timeSeconds < this->sampleArray[0].timeSeconds && trimSection == TrimSection::AFTER) ||
			(timeSeconds > this->sampleArray[this->sampleArray.size() - 1].timeSeconds && trimSection == TrimSection::BEFORE))
		{
			this->Clear();
		}
	}
	else
	{
		Sample interpolatedSample;
		this->Interpolate(sampleBounds, timeSeconds, interpolatedSample);

		uint64_t i = sampleBounds.minSample - this->sampleArray.data();
		uint64_t j = sampleBounds.maxSample - this->sampleArray.data();

		if (trimSection == TrimSection::AFTER)
		{
			this->sampleArray.resize(i);
			this->sampleArray.push_back(interpolatedSample);
		}
		else if (trimSection == TrimSection::BEFORE)
		{
			// This doesn't necessarily have any better time complexity than the regular Trim routine does.
			this->sampleArray[0] = interpolatedSample;
			for (uint64_t k = j; k < this->sampleArray.size(); k++)
				this->sampleArray[k - j + 1] = sampleArray[k];
			this->sampleArray.resize(this->sampleArray.size() - j + 1);
		}
	}
}

void WaveForm::SortSamples()
{
	std::sort(this->sampleArray.begin(), this->sampleArray.end(), [](const Sample& sampleA, const Sample& sampleB) -> bool {
		return sampleA.timeSeconds < sampleB.timeSeconds;
	});
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
		this->sampleArray.push_back(sample);
	}
}

void WaveForm::Clamp(double minAmplitude, double maxAmplitude)
{
	for (Sample& sample : this->sampleArray)
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

	double averageSamplesPerSeconds = double(this->sampleArray.size()) / timeSpanSeconds;
	return averageSamplesPerSeconds;
}

double WaveForm::CalcAverageVolume() const
{
	double averageVolume = 0.0;
	double numPeaksAndVallies = 0.0;

	for (uint32_t i = 1; i < this->sampleArray.size() - 1; i++)
	{
		const Sample& sampleA = this->sampleArray[i - 1];
		const Sample& sampleB = this->sampleArray[i];
		const Sample& sampleC = this->sampleArray[i + 1];

		double slopeA = (sampleB.amplitude - sampleA.amplitude) / (sampleB.timeSeconds - sampleA.timeSeconds);
		double slopeB = (sampleC.amplitude - sampleB.amplitude) / (sampleC.timeSeconds - sampleB.timeSeconds);

		if (ADL_SIGN(slopeA) != ADL_SIGN(slopeB))
		{
			averageVolume += ::abs(sampleB.amplitude);
			numPeaksAndVallies += 1.0;
		}
	}

	if (numPeaksAndVallies != 0.0)
		averageVolume /= numPeaksAndVallies;

	return averageVolume;
}

double WaveForm::GetStartTime() const
{
	if (this->sampleArray.size() == 0)
		return 0.0;

	return this->sampleArray[0].timeSeconds;
}

double WaveForm::GetEndTime() const
{
	if (this->sampleArray.size() == 0)
		return 0.0;

	return this->sampleArray[this->sampleArray.size() - 1].timeSeconds;
}

double WaveForm::GetTimespan() const
{
	return this->GetEndTime() - this->GetStartTime();
}

uint64_t WaveForm::GetNumSamples() const
{
	return this->sampleArray.size();
}

double WaveForm::GetMaxAmplitude() const
{
	double maxAmplitude = std::numeric_limits<double>::min();
	for (const Sample& sample : this->sampleArray)
		if (sample.amplitude > maxAmplitude)
			maxAmplitude = sample.amplitude;

	return maxAmplitude;
}

double WaveForm::GetMinAmplitude() const
{
	double minAmplitude = std::numeric_limits<double>::max();
	for (const Sample& sample : this->sampleArray)
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
	for (Sample& sample : this->sampleArray)
		sample.amplitude *= scale;
}

bool WaveForm::ContainsTime(double timeSeconds) const
{
	double startTimeSeconds = this->GetStartTime();
	double endTimeSeconds = this->GetEndTime();

	return startTimeSeconds <= timeSeconds && timeSeconds <= endTimeSeconds;
}

//---------------------------------- WaveFormStream ----------------------------------

WaveFormStream::WaveFormStream(uint32_t maxWaveForms, double maxWaveFormSizeSeconds)
{
	this->maxWaveForms = maxWaveForms;
	this->maxWaveFormSizeSeconds = maxWaveFormSizeSeconds;
}

/*virtual*/ WaveFormStream::~WaveFormStream()
{
	this->Clear();
}

void WaveFormStream::Clear()
{
	for (WaveForm* waveForm : this->waveFormList)
		delete waveForm;

	this->waveFormList.clear();
}

/*virtual*/ double WaveFormStream::EvaluateAt(double timeSeconds) const
{
	// This is effectively an O(log N) operation if this->maxWaveForms is set to 2.
	for (const WaveForm* waveForm : this->waveFormList)
		if (waveForm->ContainsTime(timeSeconds))
			return waveForm->EvaluateAt(timeSeconds);	// This is O(log N).

	return 0.0;
}

void WaveFormStream::AddSample(const WaveForm::Sample& sample)
{
	if (this->waveFormList.size() == 0)
	{
		auto waveForm = new WaveForm();
		waveForm->AddSample(sample);
		this->waveFormList.push_back(waveForm);
		return;
	}

	WaveForm* waveForm = this->waveFormList.back();
	if (waveForm->GetTimespan() < this->maxWaveFormSizeSeconds)
	{
		waveForm->AddSample(sample);
		return;
	}

	const WaveForm::Sample& lastSample = waveForm->GetSampleArray()[waveForm->GetNumSamples() - 1];

	waveForm = new WaveForm();
	waveForm->AddSample(lastSample);		// Adjacent wave-forms need to share a sample where they meet.
	waveForm->AddSample(sample);
	this->waveFormList.push_back(waveForm);

	if (this->waveFormList.size() > this->maxWaveForms)
	{
		waveForm = *this->waveFormList.begin();
		delete waveForm;
		this->waveFormList.pop_front();
	}
}

double WaveFormStream::GetDurationSeconds() const
{
	return this->GetEndTimeSeconds() - this->GetStartTimeSeconds();
}

double WaveFormStream::GetStartTimeSeconds() const
{
	if (this->waveFormList.size() == 0)
		return 0.0;

	const WaveForm* firstWaveForm = *this->waveFormList.begin();
	if (firstWaveForm->GetSampleArray().size() == 0)
		return 0.0;

	const WaveForm::Sample& firstSample = firstWaveForm->GetSampleArray()[0];
	return firstSample.timeSeconds;
}

double WaveFormStream::GetEndTimeSeconds() const
{
	if (this->waveFormList.size() == 0)
		return 0.0;

	const WaveForm* lastWaveForm = this->waveFormList.back();
	if (lastWaveForm->GetSampleArray().size() == 0)
		return 0.0;

	const WaveForm::Sample& lastSample = lastWaveForm->GetSampleArray()[lastWaveForm->GetSampleArray().size() - 1];
	return lastSample.timeSeconds;
}

bool WaveFormStream::AnyAudibleSampleFound() const
{
	for (const WaveForm* waveForm : this->waveFormList)
	{
		for (const WaveForm::Sample& sample : waveForm->GetSampleArray())
		{
			constexpr double threshold = 1e-3;	// TODO: Should this be a global define somewhere?
			if (::fabs(sample.amplitude) >= threshold)
				return true;
		}
	}

	return false;
}