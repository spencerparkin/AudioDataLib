#include "MixerModule.h"
#include "WaveForm.h"
#include "Error.h"

using namespace AudioDataLib;

MixerModule::MixerModule()
{
}

/*virtual*/ MixerModule::~MixerModule()
{
}

/*virtual*/ bool MixerModule::GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm* waveForm, std::vector<WaveForm>* waveFormsArray, Error& error)
{
	std::list<WaveForm*> waveFormList;

	for (std::shared_ptr<SynthModule>& synthModule : *this->dependentModulesArray)
	{
		if (synthModule->MoreSoundAvailable())
		{
			WaveForm* waveFormComponent = new WaveForm;
			if (synthModule->GenerateSound(durationSeconds, samplesPerSecond, waveFormComponent, nullptr, error))
				waveFormList.push_back(waveFormComponent);
			else
				delete waveFormComponent;
		}
	}

	waveForm->SumTogether(waveFormList);
	// TODO: Scale sum by 1.0 / waveFormList.size()?

	for (WaveForm* waveFormComponent : waveFormList)
		delete waveFormComponent;

	return !error;
}