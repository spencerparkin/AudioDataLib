#include "SoundFontData.h"
#include "WaveForm.h"
#include "FrequencyGraph.h"
#include "Error.h"

using namespace AudioDataLib;

//------------------------------- SoundFontData -------------------------------

SoundFontData::SoundFontData()
{
	this->generalInfo = new GeneralInfo();
	this->pitchDataArray = new std::vector<PitchData*>();
}

/*virtual*/ SoundFontData::~SoundFontData()
{
	this->Clear();

	delete this->generalInfo;
	delete this->pitchDataArray;
}

void SoundFontData::Clear()
{
	for (PitchData* pitchData : *this->pitchDataArray)
		delete pitchData;
	
	this->pitchDataArray->clear();
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
	fprintf(fp, "Num pitches: %d\n", uint32_t(this->pitchDataArray->size()));

	for (const PitchData* pitchData : *this->pitchDataArray)
	{
		Error error;
		pitchData->CalcAnalyticalPitch(error);

		fprintf(fp, "========================================\n");
		fprintf(fp, "MIDI Pitch: %d\n", pitchData->GetMIDIPitch());
		fprintf(fp, "Analytical pitch: %f\n", pitchData->GetAnalyticalPitch());
		fprintf(fp, "Channels: %d\n", pitchData->GetNumLoopedAudioDatas());

		for (uint32_t i = 0; i < pitchData->GetNumLoopedAudioDatas(); i++)
		{
			fprintf(fp, "-----------------------\n");
			const LoopedAudioData* audioData = pitchData->GetLoopedAudioData(i);
			audioData->DumpInfo(fp);
		}
	}
}

const SoundFontData::PitchData* SoundFontData::GetPitchData(uint32_t i) const
{
	if (i >= this->GetNumPitchDatas())
		return nullptr;

	return (*this->pitchDataArray)[i];
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

//------------------------------- SoundFontData::PitchData -------------------------------

SoundFontData::PitchData::PitchData()
{
	this->midiPitch = 0;
	this->loopedAudioDataArray = new std::vector<LoopedAudioData*>();
	this->analyticalPitch = 0.0;
}

/*virtual*/ SoundFontData::PitchData::~PitchData()
{
	this->Clear();

	delete this->loopedAudioDataArray;
}

void SoundFontData::PitchData::Clear()
{
	for (LoopedAudioData* audioData : *this->loopedAudioDataArray)
		delete audioData;

	this->loopedAudioDataArray->clear();
}

const SoundFontData::LoopedAudioData* SoundFontData::PitchData::GetLoopedAudioData(uint32_t i) const
{
	if (i >= this->GetNumLoopedAudioDatas())
		return nullptr;

	return (*this->loopedAudioDataArray)[i];
}

bool SoundFontData::PitchData::CalcAnalyticalPitch(Error& error) const
{
	this->analyticalPitch = 0.0;
	double maxStrength = 0.0;

	for (const LoopedAudioData* audioData : *this->loopedAudioDataArray)
	{
		const AudioData::Format& format = audioData->GetFormat();

		WaveForm waveForm;
		if (!waveForm.ConvertFromAudioBuffer(format, audioData->GetAudioBuffer(), audioData->GetAudioBufferSize(), 0, error))
			return false;

#if false		// I'm not sure that this helped at all, but rather, made things worse.
		// Might we get a more accurate reading by restricting the FFT to the looped portion of the audio?
		double loopStartTimeSeconds = format.BytesPerChannelToSeconds(audioData->GetLoop().startFrame * format.BytesPerFrame());
		double loopEndTimeSeconds = format.BytesPerChannelToSeconds(audioData->GetLoop().endFrame * format.BytesPerFrame());
		if (!waveForm.Trim(loopStartTimeSeconds, loopEndTimeSeconds, error))
			return false;
#endif

		FrequencyGraph frequencyGraph;
		if (!frequencyGraph.FromWaveForm(waveForm, 8192, error))
			return false;

		double strength = 0.0;
		double frequency = frequencyGraph.FindDominantFrequency(&strength);

		if (strength > maxStrength)
		{
			maxStrength = strength;
			this->analyticalPitch = frequency;
		}
	}
	
	return true;
}