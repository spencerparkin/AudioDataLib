#include "AudioDataLib/SynthModules/MixerModule.h"
#include "AudioDataLib/WaveForm.h"
#include "AudioDataLib/ErrorSystem.h"

using namespace AudioDataLib;

MixerModule::MixerModule()
{
}

/*virtual*/ MixerModule::~MixerModule()
{
}

/*virtual*/ bool MixerModule::GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm& waveForm, SynthModule* callingModule)
{
	std::list<WaveForm*> waveFormList;

	for (std::shared_ptr<SynthModule>& synthModule : this->dependentModulesArray)
	{
		if (synthModule->MoreSoundAvailable())
		{
			WaveForm* waveFormComponent = new WaveForm;
			if (synthModule->GenerateSound(durationSeconds, samplesPerSecond, *waveFormComponent, this))
				waveFormList.push_back(waveFormComponent);
			else
				delete waveFormComponent;
		}
	}

	waveForm.SumTogether(waveFormList);
	// TODO: Scale sum by 1.0 / waveFormList.size()?

	for (WaveForm* waveFormComponent : waveFormList)
		delete waveFormComponent;

	return !ErrorSystem::Get()->Errors();
}