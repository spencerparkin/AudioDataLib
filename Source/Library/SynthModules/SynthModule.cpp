#include "SynthModule.h"

using namespace AudioDataLib;

SynthModule::SynthModule()
{
	this->dependentModulesArray = new std::vector<std::shared_ptr<SynthModule>>();
}

/*virtual*/ SynthModule::~SynthModule()
{
	delete this->dependentModulesArray;
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
	this->dependentModulesArray->push_back(synthModule);
}

std::shared_ptr<SynthModule> SynthModule::GetDependentModule(uint32_t i)
{
	return (*this->dependentModulesArray)[i];
}

uint32_t SynthModule::GetNumDependentModules() const
{
	return this->dependentModulesArray->size();
}

void SynthModule::PruneDeadBranches()
{
	uint32_t i = 0;
	while (i < this->dependentModulesArray->size())
	{
		SynthModule* synthModule = (*this->dependentModulesArray)[i].get();
		if (synthModule->MoreSoundAvailable())
			i++;
		else
		{
			if (i < this->dependentModulesArray->size() - 1)
				(*this->dependentModulesArray)[i] = (*this->dependentModulesArray)[this->dependentModulesArray->size() - 1];

			this->dependentModulesArray->pop_back();
		}
	}
}

void SynthModule::Clear()
{
	this->dependentModulesArray->clear();
}