#include "MidiMsgDestination.h"
#include "Error.h"

using namespace AudioDataLib;

MidiMsgDestination::MidiMsgDestination()
{
}

/*virtual*/ MidiMsgDestination::~MidiMsgDestination()
{
}

/*virtual*/ bool MidiMsgDestination::ReceiveMessage(double deltaTimeSeconds, const uint8_t* message, uint64_t messageSize, Error& error)
{
	error.Add("MIDI message receiver method not overridden.");
	return false;
}

/*virtual*/ bool MidiMsgDestination::Initialize(Error& error)
{
	return true;
}

/*virtual*/ bool MidiMsgDestination::Finalize(Error& error)
{
	return true;
}

/*virtual*/ bool MidiMsgDestination::Process(Error& error)
{
	return true;
}