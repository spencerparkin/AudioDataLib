#include "InterpolationModule.h"
#include "Error.h"

using namespace AudioDataLib;

InterpolationModule::InterpolationModule()
{
}

/*virtual*/ InterpolationModule::~InterpolationModule()
{
}

/*virtual*/ bool InterpolationModule::GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm& waveForm, Error& error)
{
	if (this->GetNumDependentModules() != 2)
	{
		error.Add("Interpolation module needs exactly two dependent modules.");
		return false;
	}

	// TODO: Write this.
	error.Add("Not yet implemented.");
	return false;
}