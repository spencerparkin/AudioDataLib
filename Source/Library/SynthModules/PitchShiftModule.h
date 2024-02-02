#include "SynthModule.h"

namespace AudioDataLib
{
	class AUDIO_DATA_LIB_API PitchShiftModule : public SynthModule
	{
	public:
		PitchShiftModule();
		virtual ~PitchShiftModule();

		virtual bool GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm& waveForm, Error& error) override;
		virtual bool CantGiveAnymoreSound() override;

		void SetSourceAndTargetFrequencies(double sourceFrequency, double targetFrequency);

		double GetSourceFrequency() const { return this->sourceFrequency; }
		double GetTargetFrequency() const { return this->targetFrequency; }

		void SetDependentModule(SynthModule* synthModule) { this->dependentModule = synthModule; }
		SynthModule* GetDependentModule() { return this->dependentModule; }
		const SynthModule* GetDependentModule() const { return this->dependentModule; }

	protected:
		double sourceFrequency;
		double targetFrequency;

		SynthModule* dependentModule;
	};
}