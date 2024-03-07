#include "ALawCodec.h"

using namespace AudioDataLib;

ALawCodec::ALawCodec()
{
}

/*virtual*/ ALawCodec::~ALawCodec()
{
}

/*virtual*/ bool ALawCodec::Decode(ByteStream& inputStream, AudioData& audioOut, Error& error)
{
	return false;
}

/*virtual*/ bool ALawCodec::Encode(ByteStream& outputStream, const AudioData& audioIn, Error& error)
{
	return false;
}