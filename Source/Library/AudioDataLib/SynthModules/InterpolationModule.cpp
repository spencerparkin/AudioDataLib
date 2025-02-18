#include "AudioDataLib/SynthModules/InterpolationModule.h"
#include "AudioDataLib/ErrorSystem.h"

using namespace AudioDataLib;

InterpolationModule::InterpolationModule()
{
}

/*virtual*/ InterpolationModule::~InterpolationModule()
{
}

/*virtual*/ bool InterpolationModule::GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm& waveForm, SynthModule* callingModule)
{
	if (this->GetNumDependentModules() != 2)
	{
		ErrorSystem::Get()->Add("Interpolation module needs exactly two dependent modules.");
		return false;
	}

	// TODO: Write this.
	ErrorSystem::Get()->Add("Not yet implemented.");
	return false;
}