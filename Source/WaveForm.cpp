#include "WaveForm.h"

using namespace AudioDataLib;

//---------------------------------- WaveForm ----------------------------------

WaveForm::WaveForm()
{
	this->sampleArray = new std::vector<Sample>();
	this->index = nullptr;
}

/*virtual*/ WaveForm::~WaveForm()
{
	delete this->sampleArray;
}

void WaveForm::Clear()
{
	this->sampleArray->clear();
	delete this->index;
	this->index = nullptr;
}

uint64_t WaveForm::GetSizeBytes(const AudioData::Format& format, bool allChannels) const
{
	if (this->sampleArray->size() == 0)
		return 0;

	uint64_t numBytes = format.BytesFromSeconds(this->GetTimespan());
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
	uint64_t samplesPerFrame = format.numChannels;
	uint64_t bytesPerFrame = bytesPerSample * samplesPerFrame;

	uint64_t i = 0;
	uint32_t number = 0;
	while (i < audioBufferSize)
	{
		Sample sample;
		sample.timeSeconds = format.BytesToSeconds(i);
		sample.number = number++;

		const uint8_t* frameBuf = &audioBuffer[i];
		const uint8_t* sampleBuf = &frameBuf[bytesPerSample * channel];

		// TODO: What about endianness?
		switch (format.bitsPerSample)
		{
			case 8:
			{
				sample.amplitude = double(*reinterpret_cast<const int8_t*>(sampleBuf)) / 128.0;
				break;
			}
			case 16:
			{
				sample.amplitude = double(*reinterpret_cast<const int16_t*>(sampleBuf)) / 32768.0;
				break;
			}
			case 32:
			{
				sample.amplitude = *reinterpret_cast<const float*>(sampleBuf) / std::numeric_limits<float>::max();
				break;
			}
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

	for (const Sample& sample : *this->sampleArray)
	{
		uint64_t byteOffset = format.BytesFromSeconds(sample.timeSeconds);
		byteOffset = format.RoundDownToNearestFrameMultiple(byteOffset);
		byteOffset += bytesPerSample * channel;
		if (byteOffset + bytesPerSample > audioBufferSize)
		{
			error = "Sample-write out of buffer range.";
			return false;
		}

		// TODO: What about endianness?
		switch (format.bitsPerSample)
		{
			case 8:
			{
				int8_t sampleData = int8_t(sample.amplitude * 128.0);
				::memcpy(&audioBuffer[byteOffset], &sampleData, (size_t)bytesPerSample);
				break;
			}
			case 16:
			{
				int16_t sampleData = int16_t(sample.amplitude * 32768.0);
				::memcpy(&audioBuffer[byteOffset], &sampleData, (size_t)bytesPerSample);
				break;
			}
			case 32:
			{
				float sampleData = float(sample.amplitude * std::numeric_limits<double>::max());
				::memcpy(&audioBuffer[byteOffset], &sampleData, (size_t)bytesPerSample);
				break;
			}
		}
	}

	return true;
}

double WaveForm::EvaluateAt(double timeSeconds) const
{
	SampleBounds sampleBounds;

	if (!this->FindSampleBounds(timeSeconds, sampleBounds))
		return 0.0;

	assert(sampleBounds.minSample->number + 1 == sampleBounds.maxSample->number);
	assert(sampleBounds.minSample->timeSeconds <= timeSeconds && timeSeconds <= sampleBounds.maxSample->timeSeconds);

	double lerpAlpha = (timeSeconds - sampleBounds.minSample->timeSeconds) / (sampleBounds.maxSample->timeSeconds - sampleBounds.minSample->timeSeconds);
	double interpolatedAmplitude = sampleBounds.minSample->amplitude + lerpAlpha * (sampleBounds.maxSample->amplitude - sampleBounds.minSample->amplitude);
	return interpolatedAmplitude;
}

bool WaveForm::FindSampleBounds(double timeSeconds, SampleBounds& sampleBounds) const
{
	if (this->index)
		return this->index->FindSampleBounds(timeSeconds, sampleBounds);

	for (uint32_t i = 0; i < this->sampleArray->size() - 1; i++)
	{
		sampleBounds.minSample = &(*this->sampleArray)[i];
		sampleBounds.maxSample = &(*this->sampleArray)[i + 1];

		if (sampleBounds.minSample->timeSeconds <= timeSeconds && timeSeconds <= sampleBounds.maxSample->timeSeconds)
			return true;
	}

	return false;
}

void WaveForm::GenerateIndex() const
{
	if (this->index)
		delete this->index;

	this->index = new Index();
	this->index->Build(*this);
}

void WaveForm::SumTogether(const std::list<WaveForm*>& waveFormList)
{
	double minStartTime = std::numeric_limits<double>::max();
	double maxEndTime = std::numeric_limits<double>::min();
	double minAvgSamplesPerSecond = std::numeric_limits<double>::max();
	for (const WaveForm* waveForm : waveFormList)
	{
		double avgSamplesPerSecond = waveForm->AverageSampleRate();
		if (avgSamplesPerSecond < minAvgSamplesPerSecond)
			minAvgSamplesPerSecond = avgSamplesPerSecond;

		double startTime = waveForm->GetStartTime();
		if (startTime < minStartTime)
			minStartTime = startTime;

		double endTime = waveForm->GetEndTime();
		if (endTime > maxEndTime)
			maxEndTime = endTime;

		waveForm->GenerateIndex();
	}

	this->Clear();

	double timeSpanSeconds = maxEndTime - minStartTime;
	uint32_t numSamples = uint32_t(timeSpanSeconds * minAvgSamplesPerSecond);
	for (uint32_t i = 0; i < numSamples; i++)
	{
		Sample sample;
		sample.timeSeconds = minStartTime + (double(i) / double(numSamples - 1)) * timeSpanSeconds;
		sample.amplitude = 0.0;
		for (const WaveForm* waveForm : waveFormList)
			sample.amplitude += waveForm->EvaluateAt(sample.timeSeconds);
		this->sampleArray->push_back(sample);
	}

	this->Clamp(-1.0, 1.0);
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

//---------------------------------- WaveForm::Index ----------------------------------

WaveForm::Index::Index()
{
	this->rootNode = nullptr;
}

/*virtual*/ WaveForm::Index::~Index()
{
	this->Clear();
}

void WaveForm::Index::Clear()
{
	delete this->rootNode;
	this->rootNode = nullptr;
}

void WaveForm::Index::Build(const WaveForm& waveForm)
{
	this->Clear();

	if (waveForm.GetNumSamples() == 0)
		return;

	std::vector<Sample*> sampleArray;
	for (Sample& sample : *waveForm.sampleArray)
		sampleArray.push_back(&sample);

	this->rootNode = this->GenerateNode(sampleArray);
}

WaveForm::Index::Node* WaveForm::Index::GenerateNode(const std::vector<Sample*>& sampleArray)
{
	double startTime = sampleArray[0]->timeSeconds;
	double endTime = sampleArray[sampleArray.size() - 1]->timeSeconds;
	double midTime = (startTime + endTime) / 2.0;

	std::vector<Sample*> leftSampleArray, rightSampleArray;

	for (Sample* sample : sampleArray)
	{
		if (sample->timeSeconds < midTime)
			leftSampleArray.push_back(sample);
		else
			rightSampleArray.push_back(sample);
	}

	InternalNode* internalNode = new InternalNode(midTime);

	if (leftSampleArray.size() > 1)
		internalNode->leftNode = this->GenerateNode(leftSampleArray);
	else if(leftSampleArray.size() == 1)
		internalNode->leftNode = new LeafNode(leftSampleArray[0]);

	if (rightSampleArray.size() > 1)
		internalNode->rightNode = this->GenerateNode(rightSampleArray);
	else if (rightSampleArray.size() == 1)
		internalNode->rightNode = new LeafNode(rightSampleArray[0]);

	return internalNode;
}

bool WaveForm::Index::FindSampleBounds(double timeSeconds, SampleBounds& sampleBounds) const
{
	if (!this->rootNode)
		return false;

	return this->rootNode->FindTightestSampleBounds(timeSeconds, sampleBounds);
}

//---------------------------------- WaveForm::Index::Node ----------------------------------

WaveForm::Index::Node::Node()
{
}

/*virtual*/ WaveForm::Index::Node::~Node()
{
}

//---------------------------------- WaveForm::Index::InternalNode ----------------------------------

WaveForm::Index::InternalNode::InternalNode(double partition)
{
	this->leftNode = nullptr;
	this->rightNode = nullptr;
	this->partition = partition;
}

/*virtual*/ WaveForm::Index::InternalNode::~InternalNode()
{
	delete this->leftNode;
	delete this->rightNode;
}

/*virtual*/ bool WaveForm::Index::InternalNode::FindTightestSampleBounds(double timeSeconds, SampleBounds& sampleBounds)
{
	SampleBounds sampleBoundsLeft{ nullptr, nullptr }, sampleBoundsRight{ nullptr, nullptr };

	if (timeSeconds < this->partition)
	{
		if (this->leftNode && this->leftNode->FindTightestSampleBounds(timeSeconds, sampleBoundsLeft))
		{
			sampleBounds = sampleBoundsLeft;
			return true;
		}
	
		if (sampleBoundsLeft.minSample)
		{
			if (this->rightNode)
			{
				bool found = this->rightNode->FindTightestSampleBounds(timeSeconds, sampleBoundsRight);
				assert(!found);
			}

			if (sampleBoundsRight.maxSample)
			{
				sampleBounds.minSample = sampleBoundsLeft.minSample;
				sampleBounds.maxSample = sampleBoundsRight.maxSample;
				return true;
			}
		}
	}
	else
	{
		if (this->rightNode && this->rightNode->FindTightestSampleBounds(timeSeconds, sampleBoundsRight))
		{
			sampleBounds = sampleBoundsRight;
			return true;
		}

		if (sampleBoundsRight.maxSample)
		{
			if (this->leftNode)
			{
				bool found = this->leftNode->FindTightestSampleBounds(timeSeconds, sampleBoundsLeft);
				assert(!found);
			}

			if (sampleBoundsLeft.minSample)
			{
				sampleBounds.minSample = sampleBoundsLeft.minSample;
				sampleBounds.maxSample = sampleBoundsRight.maxSample;
				return true;
			}
		}
	}

	return false;
}

//---------------------------------- WaveForm::Index::LeafNode ----------------------------------

WaveForm::Index::LeafNode::LeafNode(Sample* sample)
{
	this->sample = sample;
}

/*virtual*/ WaveForm::Index::LeafNode::~LeafNode()
{
}

/*virtual*/ bool WaveForm::Index::LeafNode::FindTightestSampleBounds(double timeSeconds, SampleBounds& sampleBounds)
{
	if (this->sample->timeSeconds <= timeSeconds)
	{
		sampleBounds.minSample = this->sample;
		sampleBounds.maxSample = nullptr;
	}
	else
	{
		sampleBounds.minSample = nullptr;
		sampleBounds.maxSample = this->sample;
	}

	return false;
}