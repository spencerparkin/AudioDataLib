#include "InterpolationModule.h"

using namespace AudioDataLib;

InterpolationModule::InterpolationModule()
{
	this->dependentModuleA = nullptr;
	this->dependentModuleB = nullptr;
}

/*virtual*/ InterpolationModule::~InterpolationModule()
{
}

/*virtual*/ bool InterpolationModule::GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm& waveForm, Error& error)
{
	// TODO: Not sure how exactly to interpolate between two sounds for the purpose of finding an intermediate pitch.
	return false;
}