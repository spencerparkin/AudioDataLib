#include "WaveFormat.h"

using namespace AudioDataLib;

WaveFormat::WaveFormat()
{
}

/*virtual*/ WaveFormat::~WaveFormat()
{
}

/*virtual*/ bool WaveFormat::ReadFromStream(std::istream& inputStream, AudioData*& audioData, std::string& error)
{
	return false;
}

/*virtual*/ bool WaveFormat::WriteToStream(std::ostream& outputStream, const AudioData*& audioData, std::string& error)
{
	return false;
}