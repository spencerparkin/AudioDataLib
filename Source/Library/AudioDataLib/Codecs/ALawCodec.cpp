#include "AudioDataLib/Codecs/ALawCodec.h"

using namespace AudioDataLib;

ALawCodec::ALawCodec()
{
}

/*virtual*/ ALawCodec::~ALawCodec()
{
}

/*virtual*/ bool ALawCodec::Decode(ByteStream& inputStream, AudioData& audioOut)
{
	return false;
}

/*virtual*/ bool ALawCodec::Encode(ByteStream& outputStream, const AudioData& audioIn)
{
	return false;
}