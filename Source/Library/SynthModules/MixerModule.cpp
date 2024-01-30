#include "MixerModule.h"
#include "WaveForm.h"
#include "Error.h"

using namespace AudioDataLib;

MixerModule::MixerModule()
{
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
	std::vector<uint32_t> doomedKeysArray;

	for (auto pair : *this->synthModuleMap)
	{
		if (pair.second->CantGiveAnymoreSound())
		{
			doomedKeysArray.push_back(pair.first);
			delete pair.second;
		}
	}

	for (uint32_t key : doomedKeysArray)
		this->synthModuleMap->erase(key);
}

void MixerModule::SetModule(uint32_t key, SynthModule* synthModule)
{
	SynthModuleMap::iterator iter = this->synthModuleMap->find(key);
	if (iter != this->synthModuleMap->end())
	{
		delete iter->second;
		this->synthModuleMap->erase(iter);
	}

	if (synthModule != nullptr)
		this->synthModuleMap->insert(std::pair<uint32_t, SynthModule*>(key, synthModule));
}

SynthModule* MixerModule::GetModule(uint32_t key)
{
	SynthModuleMap::iterator iter = this->synthModuleMap->find(key);
	if (iter != this->synthModuleMap->end())
		return iter->second;

	return nullptr;
}