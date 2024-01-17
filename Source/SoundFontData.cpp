#include "SoundFontData.h"

using namespace AudioDataLib;

SoundFontData::SoundFontData()
{
	this->generalInfo = new GeneralInfo();
}

/*virtual*/ SoundFontData::~SoundFontData()
{
	delete this->generalInfo;
}