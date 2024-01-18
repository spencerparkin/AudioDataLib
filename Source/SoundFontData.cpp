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

/*virtual*/ void SoundFontData::DumpInfo(FILE* fp) const
{
	fprintf(fp, "Bank name: %s\n", this->generalInfo->bankName.c_str());
	fprintf(fp, "Copyright: %s\n", this->generalInfo->copyrightClaim.c_str());
	fprintf(fp, "Sound engineers: %s\n", this->generalInfo->soundEngineerNames.c_str());
	fprintf(fp, "For product: %s\n", this->generalInfo->intendedProductName.c_str());

	//...
}