#include "AudioDataLib/Error.h"

using namespace AudioDataLib;

Error::Error()
{
	this->errorArray = new std::vector<std::string>();
}

/*virtual*/ Error::~Error()
{
	delete this->errorArray;
}

void Error::Add(const std::string& error)
{
	this->errorArray->push_back(error);
}

void Error::Clear()
{
	this->errorArray->clear();
}

std::string Error::GetErrorMessage() const
{
	if (this->errorArray->size() == 0)
		return "";

	if (this->errorArray->size() == 1)
		return (*this->errorArray)[0];

	std::string message = FormatString("%d errors...\n\n", this->errorArray->size());
	for (const std::string& error : *this->errorArray)
		message += error + "\n";

	return message;
}

Error::operator bool() const
{
	return this->errorArray->size() > 0;
}

namespace AudioDataLib
{
	std::string FormatString(const char* format, ...)
	{
		char stringBuf[2 * 1024];
		va_list args;
		va_start(args, format);
		vsprintf(stringBuf, format, args);
		va_end(args);
		return stringBuf;
	}
}