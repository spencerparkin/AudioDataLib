#include "AudioDataLib/MIDI/MidiMsgDestination.h"
#include "AudioDataLib/ErrorSystem.h"

using namespace AudioDataLib;

MidiMsgDestination::MidiMsgDestination()
{
}

/*virtual*/ MidiMsgDestination::~MidiMsgDestination()
{
}

/*virtual*/ bool MidiMsgDestination::ReceiveMessage(double deltaTimeSeconds, const uint8_t* message, uint64_t messageSize)
{
	ErrorSystem::Get()->Add("MIDI message receiver method not overridden.");
	return false;
}

/*virtual*/ bool MidiMsgDestination::Initialize()
{
	return true;
}

/*virtual*/ bool MidiMsgDestination::Finalize()
{
	return true;
}

/*virtual*/ bool MidiMsgDestination::Process()
{
	return true;
}