#include "MixerModule.h"
#include "WaveForm.h"
#include "Error.h"

using namespace AudioDataLib;

MixerModule::MixerModule()
{
	this->nextModuleID = 1;
	this->synthModuleMap = new SynthModuleMap();
}

/*virtual*/ MixerModule::~MixerModule()
{
	this->Clear();

	delete this->synthModuleMap;
}

/*virtual*/ bool MixerModule::GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm& waveForm, Error& error)
{
	std::list<WaveForm*> waveFormList;

	for (auto pair : *this->synthModuleMap)
	{
		SynthModule* synthModule = pair.second;

		WaveForm* waveFormComponent = new WaveForm;
		if (synthModule->GenerateSound(durationSeconds, samplesPerSecond, *waveFormComponent, error))
			waveFormList.push_back(waveFormComponent);
		else
			delete waveFormComponent;
	}

	waveForm.SumTogether(waveFormList);
	return !error;
}

void MixerModule::Clear()
{
	for (auto pair : *this->synthModuleMap)
		delete pair.second;

	this->synthModuleMap->clear();
}

void MixerModule::PruneDeadBranches()
{
	std::vector<uint64_t> doomedIDArray;

	for (auto pair : *this->synthModuleMap)
	{
		SynthModule* synthModule = pair.second;

		if (synthModule->CantGiveAnymoreSound())
		{
			delete synthModule;
			doomedIDArray.push_back(pair.first);
		}
	}

	for (uint64_t moduleID : doomedIDArray)
		this->synthModuleMap->erase(moduleID);
}

uint64_t MixerModule::AddModule(SynthModule* synthModule)
{
	uint64_t moduleID = this->nextModuleID++;
	this->synthModuleMap->insert(std::pair<uint64_t, SynthModule*>(moduleID, synthModule));
	return moduleID;
}

bool MixerModule::RemoveModule(uint64_t moduleID)
{
	SynthModuleMap::iterator iter = this->synthModuleMap->find(moduleID);
	if (iter == this->synthModuleMap->end())
		return false;

	delete iter->second;
	this->synthModuleMap->erase(iter);
	return true;
}

SynthModule* MixerModule::FindModule(uint64_t moduleID)
{
	SynthModuleMap::iterator iter = this->synthModuleMap->find(moduleID);
	if (iter != this->synthModuleMap->end())
		return iter->second;

	return nullptr;
}