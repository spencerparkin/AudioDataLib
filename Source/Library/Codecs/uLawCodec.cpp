#include "uLawCodec.h"

using namespace AudioDataLib;

uLawCodec::uLawCodec()
{
}

/*virtual*/ uLawCodec::~uLawCodec()
{
}

/*virtual*/ bool uLawCodec::Decode(ByteStream& inputStream, AudioData& audioOut, Error& error)
{
	return false;
}

/*virtual*/ bool uLawCodec::Encode(ByteStream& outputStream, const AudioData& audioIn, Error& error)
{
	return false;
}