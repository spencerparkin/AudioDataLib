#include "SoundFontData.h"
#include "WaveForm.h"
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
	fprintf(fp, "Num pitches: %d\n", this->pitchDataArray->size());

	for (const PitchData* pitchData : *this->pitchDataArray)
	{
		Error error;
		pitchData->CalcDominantFrequency(error);

		fprintf(fp, "========================================\n");
		fprintf(fp, "MIDI Pitch: %d\n", pitchData->GetMIDIPitch());
		fprintf(fp, "Dominant Freq.: %f\n", pitchData->GetDomainantFrequency());
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
}

/*virtual*/ SoundFontData::LoopedAudioData::~LoopedAudioData()
{
	delete this->name;
}

/*virtual*/ void SoundFontData::LoopedAudioData::DumpInfo(FILE* fp) const
{
	fprintf(fp, "Name: %s\n", this->name->c_str());
	fprintf(fp, "Loop start frame: %lld\n", this->loop.startFrame);
	fprintf(fp, "Loop end frame: %lld\n", this->loop.endFrame);
	fprintf(fp, "Num frames: %lld\n", this->GetNumFrames());

	AudioData::DumpInfo(fp);
}

//------------------------------- SoundFontData::PitchData -------------------------------

SoundFontData::PitchData::PitchData()
{
	this->midiPitch = 0;
	this->dominantFrequency = 0.0;
	this->loopedAudioDataArray = new std::vector<LoopedAudioData*>();
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

bool SoundFontData::PitchData::CalcDominantFrequency(Error& error) const
{
	std::vector<double> dominantFrequenciesByChannelArray;

	for (const LoopedAudioData* audioData : *this->loopedAudioDataArray)
	{
		WaveForm waveForm;
		if (!waveForm.ConvertFromAudioBuffer(audioData->GetFormat(), audioData->GetAudioBuffer(), audioData->GetAudioBufferSize(), 0, error))
			return false;

		std::list<double> dominantFrequenciesList;
		if (!waveForm.CalcDominantFrequencies(dominantFrequenciesList, 1, error))
			return false;

		dominantFrequenciesByChannelArray.push_back(*dominantFrequenciesList.begin());
	}
	
	if (dominantFrequenciesByChannelArray.size() == 0)
	{
		error.Add("Was unable to calculate the dominant frequency for any channel.");
		return false;
	}

	double averageFrequency = 0.0;
	for (double frequency : dominantFrequenciesByChannelArray)
		averageFrequency += frequency;

	averageFrequency /= double(dominantFrequenciesByChannelArray.size());

	this->dominantFrequency = averageFrequency;

	return true;
}