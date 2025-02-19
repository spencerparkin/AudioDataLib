#include "AudioDataLib/SynthModules/SynthModule.h"

using namespace AudioDataLib;

SynthModule::SynthModule()
{
}

/*virtual*/ SynthModule::~SynthModule()
{
}

/*virtual*/ bool SynthModule::MoreSoundAvailable()
{
	// Some modules are "fire and forget".  So this method can be
	// called periodically until the caller knows it can prune
	// the module from the dependency graph.
	return true;
}

void SynthModule::AddDependentModule(std::shared_ptr<SynthModule> synthModule)
{
	// TODO: Check for circular reference and, if found, return an error?
	this->dependentModulesArray.push_back(synthModule);
}

std::shared_ptr<SynthModule> SynthModule::GetDependentModule(uint32_t i)
{
	return this->dependentModulesArray[i];
}

uint32_t SynthModule::GetNumDependentModules() const
{
	return this->dependentModulesArray.size();
}

void SynthModule::PruneDeadBranches()
{
	uint32_t i = 0;
	while (i < this->dependentModulesArray.size())
	{
		SynthModule* synthModule = this->dependentModulesArray[i].get();
		if (synthModule->MoreSoundAvailable())
			i++;
		else
		{
			if (i < this->dependentModulesArray.size() - 1)
				this->dependentModulesArray[i] = this->dependentModulesArray[this->dependentModulesArray.size() - 1];

			this->dependentModulesArray.pop_back();
		}
	}
}

void SynthModule::Clear()
{
	this->dependentModulesArray.clear();
}