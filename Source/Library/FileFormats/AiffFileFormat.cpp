#include "AiffFileFormat.h"
#include "Error.h"

using namespace AudioDataLib;

AiffFileFormat::AiffFileFormat()
{
}

/*virtual*/ AiffFileFormat::~AiffFileFormat()
{
}

/*virtual*/ bool AiffFileFormat::ReadFromStream(ByteStream& inputStream, FileData*& fileData, Error& error)
{
	error.Add("Not yet implemented.");
	return false;
}

/*virtual*/ bool AiffFileFormat::WriteToStream(ByteStream& outputStream, const FileData* fileData, Error& error)
{
	error.Add("Not yet implemented.");
	return false;
}