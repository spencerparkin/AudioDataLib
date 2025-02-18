#include "AudioDataLib/ErrorSystem.h"

using namespace AudioDataLib;

ErrorSystem::ErrorSystem()
{
}

/*virtual*/ ErrorSystem::~ErrorSystem()
{
}

/*static*/ ErrorSystem* ErrorSystem::Get()
{
	static ErrorSystem errorSystem;
	return &errorSystem;
}

void ErrorSystem::Add(const std::string& error)
{
	this->errorArray.push_back(error);
}

void ErrorSystem::Clear()
{
	this->errorArray.clear();
}

bool ErrorSystem::Errors()
{
	return this->errorArray.size() > 0;
}

std::string ErrorSystem::GetErrorMessage()
{
	if (this->errorArray.size() == 0)
		return "";

	if (this->errorArray.size() == 1)
		return this->errorArray[0];

	std::string message = std::format("{} errors...\n\n", this->errorArray.size());
	for (const std::string& error : this->errorArray)
		message += error + "\n";

	return message;
}