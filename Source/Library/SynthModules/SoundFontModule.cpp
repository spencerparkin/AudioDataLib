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

	// TODO: Maybe this module should not know anything about interpolation of pitch shifting.  Maybe all
	//       it should do is just know how to play back an assigned sample and loop it without artifacting.
	//       Some other module can receive the sound and pitch shift it as desired, or two instances of this
	//       module could be fed into an interpolating module to get the correct pitch.  In any case, making
	//       this module as simple as possible is helpful and an easy first step.

	return false;
}