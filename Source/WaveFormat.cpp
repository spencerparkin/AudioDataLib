#include "WaveFormat.h"
#include "ByteStream.h"

using namespace AudioDataLib;

WaveFormat::WaveFormat()
{
}

/*virtual*/ WaveFormat::~WaveFormat()
{
}

/*virtual*/ bool WaveFormat::ReadFromStream(ByteStream& inputStream, AudioData*& audioData, std::string& error)
{
	return false;
}

/*virtual*/ bool WaveFormat::WriteToStream(ByteStream& outputStream, AudioData* audioData, std::string& error)
{
	return false;
}