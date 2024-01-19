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

	//...
}

SoundFontData::AudioSample::AudioSample()
{
	this->audioData = new AudioData();
}

/*virtual*/ SoundFontData::AudioSample::~AudioSample()
{
	delete this->audioData;
}