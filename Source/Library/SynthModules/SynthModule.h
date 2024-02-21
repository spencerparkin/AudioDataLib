#pragma once

#include "Common.h"

namespace AudioDataLib
{
	class WaveForm;
	class Error;

	// TODO: It has just occurred to me that, although it was intended, it is not possible
	//       for two different modules to depend upon the same module.  I can think of a
	//       way to do it, but it would really require a major rework of the entire module
	//       system.  Instead of a single call to GenerateSound() recursing through a tree,
	//       we might need an algorithm that evaulates a dependency graph until all root
	//       nodes have the needed sound.  I think you would also need some special stream-like
	//       data-structures to handle the communication between the nodes.  Also, I think I
	//       can make this possible with what I have now, but it would require a special module
	//       specifically designed to take a single input and produce multiple copies of that
	//       input for multiple outputs.  An additional parameter would be need for the module
	//       to know "who's asking" for the sound.  This might not be an unreasonable approach,
	//       since the more generalized solution I was thinking of may be overkill when in most
	//       cases, what I already have here is sofficient, and no graph algorithm is needed;
	//       we can still just make a single call to GenerateSound() that recursive the graph.
	//       When the said node is encountered, it can use its cache to produce a result.

	// These are the building-blocks of sound synthesis.
	class AUDIO_DATA_LIB_API SynthModule
	{
	public:
		SynthModule();
		virtual ~SynthModule();

		// Note that by convention, the returned wave-form should always be zero-based with respect to time.
		// It's important to understand this when trying to make a synth module that maintains its own sense
		// of continuity, call to call.  Typically, a module generates just a single wave-form by populating
		// the samples for the single given wave-form pointer.  If, however, a module returns two or more
		// wave-form results, then those can be put into the given array, if any.  It's up to the module to
		// decide what it returns and how the results are to be interpreted.
		virtual bool GenerateSound(double durationSeconds, double samplesPerSecond, WaveForm* waveForm, std::vector<WaveForm>* waveFormsArray, Error& error) = 0;
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