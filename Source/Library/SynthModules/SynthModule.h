#pragma once

#include "Common.h"

namespace AudioDataLib
{
	class WaveForm;
	class Error;

	// These are the building-blocks of sound synthesis.
	class AUDIO_DATA_LIB_API SynthModule
	{
	public:
		SynthModule();
		virtual ~SynthModule();

		// Note that by convention, the returned wave-form should always be zero-based with respect to time.
		// It's important to understand this when trying to make a synth module that maintains its own sense
		// of continuity, call to call.
		virtual bool GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm& waveForm, Error& error) = 0;
		virtual bool MoreSoundAvailable();

		void AddDependentModule(std::shared_ptr<SynthModule> synthModule);
		std::shared_ptr<SynthModule> GetDependentModule(uint32_t i);
		uint32_t GetNumDependentModules() const;
		void PruneDeadBranches();
		void Clear();

		template<typename T>
		T* FindModule()
		{
			T* foundModule = dynamic_cast<T*>(this);
			if (foundModule)
				return foundModule;

			for (std::shared_ptr<SynthModule>& synthModule : *this->dependentModulesArray)
			{
				foundModule = synthModule->FindModule<T>();
				if (foundModule)
					return foundModule;
			}

			return nullptr;
		}

	protected:

		// Shared pointers are a reasonable choice here, because the life
		// of a module can extend beyond the desire to own it in some of
		// the calling code, such as when a key is released, but a sound is
		// still reverberating.  Further, shared pointers can also make it
		// possible to cross-pollinate between channels, e.g., for the purpose
		// of implementing pan.
		std::vector<std::shared_ptr<SynthModule>>* dependentModulesArray;
	};
}