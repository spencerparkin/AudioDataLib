#include "SoundFontData.h"
#include "MIDI/MidiSynth.h"
#include "WaveForm.h"
#include "Error.h"

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
	fprintf(fp, "Sample Name, Org. Key, Over. Key, Over. Freq., Est. Freq., Min. Freq., Max. Freq., Est. Vol., Min. Vol., Max. Vol.\n");

	for (const AudioSample* audioSample : *this->audioSampleArray)
	{
		for (uint32_t i = 0; i < audioSample->GetNumLoopedAudioDatas(); i++)
		{
			const LoopedAudioData* audioData = audioSample->GetLoopedAudioData(i);
			const LoopedAudioData::MetaData& metaData = audioData->GetMetaData();
			const LoopedAudioData::Location& location = audioData->GetLocation();
			const LoopedAudioData::MidiKeyInfo& keyInfo = audioData->GetMidiKeyInfo();

			Error error;
			audioData->CalcMetaData(error);

			double minFreq = MidiSynth::MidiPitchToFrequency(location.minKey);
			double maxFreq = MidiSynth::MidiPitchToFrequency(location.maxKey);

			double minVol = MidiSynth::MidiVelocityToAmplitude(location.minVel);
			double maxVol = MidiSynth::MidiVelocityToAmplitude(location.maxVel);

			fprintf(fp, "%s, %d, %d, %f, %f, %f, %f, %f, %f, %f\n",
				audioData->GetName().c_str(),
				keyInfo.original, keyInfo.overridingRoot,
				MidiSynth::MidiPitchToFrequency(keyInfo.overridingRoot),
				metaData.pitch, minFreq, maxFreq,
				metaData.volume, minVol, maxVol);
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
			const LoopedAudioData::MetaData& metaData = audioData->GetMetaData();

			double deltaPitch = metaData.pitch - pitch;
			double deltaVol = metaData.volume - volume;

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

const SoundFontData::AudioSample* SoundFontData::FindRelevantAudioSample(uint16_t midiKey, uint16_t midiVelocity) const
{
	for (const AudioSample* audioSample : *this->audioSampleArray)
	{
		uint32_t j = 0;

		for (uint32_t i = 0; i < audioSample->GetNumLoopedAudioDatas(); i++)
		{
			const LoopedAudioData* audioData = audioSample->GetLoopedAudioData(i);
			if (audioData->GetLocation().Contains(midiKey, midiVelocity))
				j++;
		}

		if (j == audioSample->GetNumLoopedAudioDatas())
			return audioSample;
	}

	return nullptr;
}

SoundFontData::LoopedAudioData* SoundFontData::FindLoopedAudioData(uint32_t sampleID)
{
	for (AudioSample* audioSample : *this->audioSampleArray)
		for (LoopedAudioData* audioData : audioSample->GeLoopedAudioDataArray())
			if (audioData->GetSampleID() == sampleID)
				return audioData;

	return nullptr;
}

//------------------------------- SoundFontData::LoopedAudioData -------------------------------

SoundFontData::LoopedAudioData::LoopedAudioData()
{
	this->cachedWaveForm = new std::shared_ptr<WaveForm>();
	this->sampleID = 0;
	this->loop.startFrame = 0;
	this->loop.endFrame = 0;
	this->name = new std::string();
	this->channelType = ChannelType::MONO;
	this->mode = Mode::GETS_TRAPPED_IN_LOOP;
	this->keyInfo.original = -1;
	this->keyInfo.overridingRoot = -1;
	::memset(&this->location, 0, sizeof(location));
}

/*virtual*/ SoundFontData::LoopedAudioData::~LoopedAudioData()
{
	delete this->cachedWaveForm;
	delete this->name;
}

std::shared_ptr<WaveForm> SoundFontData::LoopedAudioData::GetCachedWaveForm(uint16_t channel, Error& error) const
{
	if (!this->cachedWaveForm->get())
	{
		this->cachedWaveForm->reset(new WaveForm());

		if (!(*this->cachedWaveForm)->ConvertFromAudioBuffer(this->GetFormat(), this->GetAudioBuffer(), this->GetAudioBufferSize(), channel, error))
		{
			error.Add("Failed to convert looped audio buffer into a wave-form.");
			this->cachedWaveForm->reset();
		}
	}

	return *this->cachedWaveForm;
}

bool SoundFontData::LoopedAudioData::Location::Contains(uint16_t key, uint16_t vel) const
{
	if (!(this->minKey <= key && key <= this->maxKey))
		return false;

	if (this->minVel == this->maxVel)
		return true;

	return this->minVel <= vel && vel <= this->maxVel;
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

	const char* modeStr = "?";

	switch (this->mode)
	{
	case Mode::GETS_TRAPPED_IN_LOOP:
		modeStr = "Gets trapped in looped.";
		break;
	case Mode::EXIT_LOOP_ON_RELEASE:
		modeStr = "Exits loop on release.";
		break;
	case Mode::NOT_LOOPED:
		modeStr = "Not looped.";
		break;
	}

	fprintf(fp, "Name: %s\n", this->name->c_str());
	fprintf(fp, "Loop start frame: %lld\n", this->loop.startFrame);
	fprintf(fp, "Loop end frame: %lld\n", this->loop.endFrame);
	fprintf(fp, "Num frames: %lld\n", this->GetNumFrames());
	fprintf(fp, "Channel type: %s\n", channelTypeStr);
	fprintf(fp, "Mode: %s\n", modeStr);
	fprintf(fp, "Min/max keys: [%d, %d]\n", this->location.minKey, this->location.maxKey);
	fprintf(fp, "Min/max vels: [%d, %d]\n", this->location.minVel, this->location.maxVel);

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