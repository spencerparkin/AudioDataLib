#include "MidiData.h"

using namespace AudioDataLib;

MidiData::MidiData()
{
}

/*virtual*/ MidiData::~MidiData()
{
}

/*static*/ MidiData* MidiData::Create()
{
	return new MidiData();
}

/*static*/ void MidiData::Destroy(MidiData* midiData)
{
	delete midiData;
}