#include "SynthModule.h"

namespace AudioDataLib
{
	class AUDIO_DATA_LIB_API PitchShiftModule : public SynthModule
	{
	public:
		PitchShiftModule();
		virtual ~PitchShiftModule();

		virtual bool GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm& waveForm, Error& error) override;
	};
}