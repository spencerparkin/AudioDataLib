#include "SoundFontData.h"

using namespace AudioDataLib;

//------------------------------- SoundFontData -------------------------------

SoundFontData::SoundFontData()
{
	this->generalInfo = new GeneralInfo();
	this->audioSampleArray = new std::vector<AudioSample*>();
}

/*virtual*/ SoundFontData::~SoundFontData()
{
	this->Clear();

	delete this->generalInfo;
	delete this->audioSampleArray;
}

void SoundFontData::Clear()
{
	for (AudioSample* audioSample : *this->audioSampleArray)
		delete audioSample;
	
	this->audioSampleArray->clear();
}

/*virtual*/ FileData* SoundFontData::Clone() const
{
	return nullptr;
}

/*virtual*/ void SoundFontData::DumpInfo(FILE* fp) const
{
	fprintf(fp, "Bank name: %s\n", this->generalInfo->bankName.c_str());
	fprintf(fp, "Copyright: %s\n", this->generalInfo->copyrightClaim.c_str());
	fprintf(fp, "Sound engineers: %s\n", this->generalInfo->soundEngineerNames.c_str());
	fprintf(fp, "For product: %s\n", this->generalInfo->intendedProductName.c_str());
	fprintf(fp, "Comments: %s\n", this->generalInfo->comments.c_str());
	fprintf(fp, "Creation date: %s\n", this->generalInfo->creationDate.c_str());
	fprintf(fp, "Tool: %s\n", this->generalInfo->soundFontToolRecord.c_str());
	fprintf(fp, "Wavetable engine: %s\n", this->generalInfo->waveTableSoundEngine.c_str());
	fprintf(fp, "Wavetable ROM: %s\n", this->generalInfo->waveTableSoundDataROM.c_str());
	fprintf(fp, "Num audio samples: %d\n", uint32_t(this->audioSampleArray->size()));

	for (const AudioSample* audioSample : *this->audioSampleArray)
	{
		fprintf(fp, "========================================\n");
		fprintf(fp, "MIDI Pitch: %d\n", audioSample->GetMIDIPitch());
		fprintf(fp, "Channels: %d\n", audioSample->GetNumLoopedAudioDatas());

		for (uint32_t i = 0; i < audioSample->GetNumLoopedAudioDatas(); i++)
		{
			fprintf(fp, "-----------------------\n");
			const LoopedAudioData* audioData = audioSample->GetLoopedAudioData(i);
			audioData->DumpInfo(fp);
		}
	}
}

/*virtual*/ void SoundFontData::DumpCSV(FILE* fp) const
{
	fprintf(fp, "Sample Name, Frequency, Volume\n");

	for (const AudioSample* audioSample : *this->audioSampleArray)
	{
		for (uint32_t i = 0; i < audioSample->GetNumLoopedAudioDatas(); i++)
		{
			const LoopedAudioData* audioData = audioSample->GetLoopedAudioData(i);
			const LoopedAudioData::MetaData* metaData = audioData->GetMetaData();

			fprintf(fp, "%s, %f, %f\n", audioData->GetName().c_str(), metaData->analyticalPitch, metaData->analyticalVolume);
		}
	}
}

const SoundFontData::AudioSample* SoundFontData::GetAudioSample(uint32_t i) const
{
	if (i >= this->GetNumAudioSamples())
		return nullptr;

	return (*this->audioSampleArray)[i];
}

const SoundFontData::AudioSample* SoundFontData::FindClosestAudioSample(double pitch, double volume) const
{
	const AudioSample* closestAudioSample = nullptr;
	double smallestDistance = std::numeric_limits<double>::max();
	for (const AudioSample* audioSample : *this->audioSampleArray)
	{
		for (uint32_t i = 0; i < audioSample->GetNumLoopedAudioDatas(); i++)
		{
			const LoopedAudioData* audioData = audioSample->GetLoopedAudioData(i);
			const LoopedAudioData::MetaData* metaData = audioData->GetMetaData();

			double deltaPitch = metaData->analyticalPitch - pitch;
			double deltaVol = metaData->analyticalVolume - volume;

			double distance = deltaPitch * deltaPitch + deltaVol * deltaVol;

			if (distance < smallestDistance)
			{
				smallestDistance = distance;
				closestAudioSample = audioSample;
			}
		}
	}

	return closestAudioSample;
}

//------------------------------- SoundFontData::LoopedAudioData -------------------------------

SoundFontData::LoopedAudioData::LoopedAudioData()
{
	this->loop.startFrame = 0;
	this->loop.endFrame = 0;
	this->name = new std::string();
	this->channelType = ChannelType::MONO;
}

/*virtual*/ SoundFontData::LoopedAudioData::~LoopedAudioData()
{
	delete this->name;
}

/*virtual*/ void SoundFontData::LoopedAudioData::DumpInfo(FILE* fp) const
{
	const char* channelTypeStr = "?";

	switch (this->channelType)
	{
	case ChannelType::MONO:
		channelTypeStr = "Mono";
		break;
	case ChannelType::LEFT_EAR:
		channelTypeStr = "Left ear";
		break;
	case ChannelType::RIGHT_EAR:
		channelTypeStr = "Right ear";
		break;
	}

	fprintf(fp, "Name: %s\n", this->name->c_str());
	fprintf(fp, "Loop start frame: %lld\n", this->loop.startFrame);
	fprintf(fp, "Loop end frame: %lld\n", this->loop.endFrame);
	fprintf(fp, "Num frames: %lld\n", this->GetNumFrames());
	fprintf(fp, "Channel type: %s\n", channelTypeStr);

	AudioData::DumpInfo(fp);
}

/*virtual*/ FileData* SoundFontData::LoopedAudioData::Clone() const
{
	auto loopedAudioData = new LoopedAudioData();
	loopedAudioData->loop = this->loop;
	loopedAudioData->SetName(this->GetName());
	loopedAudioData->SetAudioBufferSize(this->GetAudioBufferSize());
	::memcpy(loopedAudioData->GetAudioBuffer(), this->GetAudioBuffer(), (size_t)loopedAudioData->GetAudioBufferSize());
	loopedAudioData->SetFormat(this->GetFormat());
	return loopedAudioData;
}

//------------------------------- SoundFontData::AudioSample -------------------------------

SoundFontData::AudioSample::AudioSample()
{
	this->midiPitch = 0;
	this->loopedAudioDataArray = new std::vector<LoopedAudioData*>();
}

/*virtual*/ SoundFontData::AudioSample::~AudioSample()
{
	this->Clear();

	delete this->loopedAudioDataArray;
}

void SoundFontData::AudioSample::Clear()
{
	for (LoopedAudioData* audioData : *this->loopedAudioDataArray)
		delete audioData;

	this->loopedAudioDataArray->clear();
}

const SoundFontData::LoopedAudioData* SoundFontData::AudioSample::GetLoopedAudioData(uint32_t i) const
{
	if (i >= this->GetNumLoopedAudioDatas())
		return nullptr;

	return (*this->loopedAudioDataArray)[i];
}

const SoundFontData::LoopedAudioData* SoundFontData::AudioSample::FindLoopedAudioData(LoopedAudioData::ChannelType channelType) const
{
	for (const LoopedAudioData* audioData : *this->loopedAudioDataArray)
		if (audioData->GetChannelType() == channelType)
			return audioData;

	return nullptr;
}