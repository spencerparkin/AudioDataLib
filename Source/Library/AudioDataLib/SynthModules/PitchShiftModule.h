#include "AudioDataLib/SynthModules/SynthModule.h"

namespace AudioDataLib
{
	class AUDIO_DATA_LIB_API PitchShiftModule : public SynthModule
	{
	public:
		PitchShiftModule();
		virtual ~PitchShiftModule();

		virtual bool GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm& waveForm, SynthModule* callingModule) override;
		virtual bool MoreSoundAvailable() override;

		void SetSourceAndTargetFrequencies(double sourceFrequency, double targetFrequency);

		double GetSourceFrequency() const { return this->sourceFrequency; }
		double GetTargetFrequency() const { return this->targetFrequency; }

	protected:
		double sourceFrequency;
		double targetFrequency;
	};
}