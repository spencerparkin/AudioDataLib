#pragma once

#include "SynthModule.h"

namespace AudioDataLib
{
	class AUDIO_DATA_LIB_API MixerModule : public SynthModule
	{
	public:
		MixerModule();
		virtual ~MixerModule();

		virtual bool GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm& waveForm, Error& error) override;

		typedef std::map<uint32_t, SynthModule*> SynthModuleMap;

		void Clear();

		void SetModule(uint32_t key, SynthModule* synthModule);
		SynthModule* GetModule(uint32_t key);

		const SynthModuleMap& GetModuleMap() const { return *this->synthModuleMap; }

	protected:
		SynthModuleMap* synthModuleMap;
	};
}