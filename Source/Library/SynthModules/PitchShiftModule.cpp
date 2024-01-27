#include "PitchShiftModule.h"

using namespace AudioDataLib;

PitchShiftModule::PitchShiftModule()
{
}

/*virtual*/ PitchShiftModule::~PitchShiftModule()
{
}

/*virtual*/ bool PitchShiftModule::GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm& waveForm, Error& error)
{
	// TODO: Here we ask for more or less sound (in terms of duration) from our dependent module and then
	//       resample it in a way that stretches out or compresses the wave-form, effectively increasing
	//       or decreasing the pitch of the sound.  (Think of playing back a recording too fast or too
	//       slow.)  The primary application of doing this is to modify the pitch of looped audio so that
	//       we can play pitches not pre-recorded in a sound-font.  Note that we could probably only pitch
	//       shift so far before we would need to resort to interpolation.  This would be handled by a different
	//       module that takes two dependent modules as inputs and then interpolates between them.  I am as
	//       yet unsure how interpolation can be done properly between two given wave forms.
	return false;
}