#include "SoundFontData.h"

using namespace AudioDataLib;

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
	fprintf(fp, "Num samples: %d\n", this->audioSampleArray->size());

	for (const AudioSample* audioSample : *this->audioSampleArray)
	{
		fprintf(fp, "========================================\n");
		fprintf(fp, "Name: %s\n", audioSample->GetName().c_str());
		fprintf(fp, "Loop frame start: %lld\n", audioSample->GetLoop().startFrame);
		fprintf(fp, "Loop frame end: %lld\n", audioSample->GetLoop().endFrame);
		fprintf(fp, "Total frames: %lld\n", audioSample->GetAudioData()->GetNumFrames());
		const AudioData* audioData = audioSample->GetAudioData();
		audioData->DumpInfo(fp);
	}
}

const SoundFontData::AudioSample* SoundFontData::GetAudioSample(uint32_t i) const
{
	if (i >= this->GetNumAudioSamples())
		return nullptr;

	return (*this->audioSampleArray)[i];
}

SoundFontData::AudioSample::AudioSample()
{
	this->audioData = new AudioData();
}

/*virtual*/ SoundFontData::AudioSample::~AudioSample()
{
	delete this->audioData;
}