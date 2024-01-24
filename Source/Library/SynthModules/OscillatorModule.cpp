#include "OscillatorModule.h"

using namespace AudioDataLib;

OscillatorModule::OscillatorModule()
{
}

/*virtual*/ OscillatorModule::~OscillatorModule()
{
}

/*virtual*/ bool OscillatorModule::GenerateSound(double timeSeconds, double samplesPerSecond, WaveForm& waveForm)
{
	return false;
}