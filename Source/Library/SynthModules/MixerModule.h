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

		void Clear();

		uint64_t AddModule(SynthModule* synthModule);
		bool RemoveModule(uint64_t moduleID);
		SynthModule* FindModule(uint64_t moduleID);

		typedef std::map<uint64_t, SynthModule*> SynthModuleMap;

		const SynthModuleMap& GetModuleMap() const { return *this->synthModuleMap; }

		void PruneDeadBranches();

	protected:
		SynthModuleMap* synthModuleMap;
		uint64_t nextModuleID;
	};
}