#include "SoundFontModule.h"

using namespace AudioDataLib;

SoundFontModule::SoundFontModule()
{
	this->soundFontData = nullptr;
}

/*virtual*/ SoundFontModule::~SoundFontModule()
{
}

/*virtual*/ bool SoundFontModule::GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm& waveForm, Error& error)
{
	// TODO: The desired pitch can be achieved by either interpolating between adjacent recordings, or
	//       perhaps by over or under-sampling the recording.  (A recording played back faster sounds
	//       higher in pitch, or played back slower sounds lower in pitch.)

	return false;
}