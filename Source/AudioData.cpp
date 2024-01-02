#include "AudioData.h"

using namespace AudioDataLib;

AudioData::AudioData()
{
	::memset(&this->format, 0, sizeof(Format));
}

/*virtual*/ AudioData::~AudioData()
{
}

AudioData* AudioData::Clone() const
{
	return nullptr;
}