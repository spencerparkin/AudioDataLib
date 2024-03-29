#include "CmdLineParser.h"
#include <algorithm>

CmdLineParser::CmdLineParser()
{
}

/*virtual*/ CmdLineParser::~CmdLineParser()
{
}

void CmdLineParser::RegisterArg(const std::string& argName, int numValues, const std::string& argHelp)
{
	ArgDef argDef;
	argDef.name = "--" + argName;
	argDef.help = argHelp;
	argDef.numValues = numValues;
	
	this->argDefArray.push_back(argDef);
}

void CmdLineParser::PrintUsage(FILE* fp) const
{
	fprintf(fp, "Synopsis: ");
	fprintf(fp, this->synopsis.c_str());
	fprintf(fp, "\n\n");
	fprintf(fp, "Usage...\n\n");

	std::vector<const ArgDef*> sortableArgDefArray;
	for (const ArgDef& argDef : this->argDefArray)
		sortableArgDefArray.push_back(&argDef);

	std::sort(sortableArgDefArray.begin(), sortableArgDefArray.end(), [](const ArgDef* argDefA, const ArgDef* argDefB) -> bool {
		return ::strcmp(argDefA->name.c_str(), argDefB->name.c_str()) < 0;
	});

	for (const ArgDef* argDef : sortableArgDefArray)
	{
		fprintf(fp, "%s (followed by %d value(s))", argDef->name.c_str(), argDef->numValues);
		fprintf(fp, "\n\t%s\n", argDef->help.c_str());
	}

	fprintf(fp, "\n");
}

const CmdLineParser::ArgDef* CmdLineParser::FindArgDef(const std::string& argName) const
{
	for (const ArgDef& argDef : this->argDefArray)
		if (argDef.name == argName)
			return &argDef;

	return nullptr;
}

const CmdLineParser::ArgInst* CmdLineParser::FindArgInst(const std::string& argName, int instance /*= 0*/) const
{
	for (const ArgInst& argInst : this->argInstArray)
		if (argInst.argDef->name == "--" + argName && instance-- == 0)
			return &argInst;

	return nullptr;
}

bool CmdLineParser::ArgGiven(const std::string& argName, int instance) const
{
	return this->FindArgInst(argName, instance) != nullptr;
}

const std::string& CmdLineParser::GetArgValue(const std::string& argName, int valueOffset, int instance /*= 0*/)
{
	static std::string emptyValue;

	const ArgInst* argInst = this->FindArgInst(argName, instance);
	if (!argInst)
		return emptyValue;

	if (0 <= valueOffset && valueOffset < (int)argInst->valueArray.size())
		return argInst->valueArray[valueOffset];

	return emptyValue;
}

bool CmdLineParser::Parse(int argc, char** argv, std::string& error)
{
	this->argInstArray.clear();

	for (int i = 1; i < argc; i++)
	{
		std::string argName(argv[i]);
		const ArgDef* argDef = this->FindArgDef(argName);
		if (!argDef)
		{
			error = "Argument \"" + argName + "\" not recognized.";
			return false;
		}

		ArgInst argInst;
		argInst.argDef = argDef;

		for (int j = 1; j <= argDef->numValues; j++)
		{
			if (i + j >= argc)
			{
				char errorBuf[128];
				sprintf_s(errorBuf, sizeof(errorBuf), "Argument \"%s\" needed %d value(s).", argName.c_str(), argDef->numValues);
				error = errorBuf;
				return false;
			}

			std::string argValue = argv[i + j];
			if (argValue.find("--") == 0)
			{
				char errorBuf[128];
				sprintf_s(errorBuf, sizeof(errorBuf), "Argument \"%s\" didn't get value %d of %d.", argName.c_str(), j, argDef->numValues);
				error = errorBuf;
				return false;
			}

			argInst.valueArray.push_back(argValue);
		}

		this->argInstArray.push_back(argInst);
		i += argDef->numValues;
	}

	return true;
}