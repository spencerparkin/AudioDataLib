#include "SynthModule.h"

using namespace AudioDataLib;

SynthModule::SynthModule()
{
}

/*virtual*/ SynthModule::~SynthModule()
{
}

/*virtual*/ bool SynthModule::MoreSoundAvailable()
{
	// Some modules are "fire and forget".  So this method can be
	// called periodically until the caller knows it can prune
	// the module from the dependency graph.
	return true;
}