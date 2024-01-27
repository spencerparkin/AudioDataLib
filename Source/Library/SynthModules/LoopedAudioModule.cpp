#include "LoopedAudioModule.h"

using namespace AudioDataLib;

LoopedAudioModule::LoopedAudioModule()
{
	this->loopedAudioData = nullptr;
}

/*virtual*/ LoopedAudioModule::~LoopedAudioModule()
{
}

/*virtual*/ bool LoopedAudioModule::GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm& waveForm, Error& error)
{
	return false;
}