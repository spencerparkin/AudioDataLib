#pragma once

#include <string>
#include <vector>

class CmdLineParser
{
public:
	CmdLineParser();
	virtual ~CmdLineParser();

	void RegisterArg(const std::string& argName, int numValues, const std::string& argHelp);
	void PrintUsage(FILE* fp) const;
	bool Parse(int argc, char** argv, std::string& error);
	bool ArgGiven(const std::string& argName, int instance = 0) const;
	const std::string& GetArgValue(const std::string& argName, int valueOffset, int instance = 0);
	void SetSynopsis(const std::string& synopsis) { this->synopsis = synopsis; }

private:
	struct ArgDef
	{
		std::string name;
		std::string help;
		int numValues;
	};

	struct ArgInst
	{
		const ArgDef* argDef;
		std::vector<std::string> valueArray;
	};

	const ArgDef* FindArgDef(const std::string& argName) const;
	const ArgInst* FindArgInst(const std::string& argName, int instance) const;

	std::vector<ArgDef> argDefArray;
	std::vector<ArgInst> argInstArray;
	std::string synopsis;
};