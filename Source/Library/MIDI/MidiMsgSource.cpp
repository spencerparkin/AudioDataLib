#include "MidiMsgSource.h"
#include "MidiMsgDestination.h"
#include "Error.h"

using namespace AudioDataLib;

MidiMsgSource::MidiMsgSource()
{
	this->destinationArray = new std::vector<MidiMsgDestination*>();
}

/*virtual*/ MidiMsgSource::~MidiMsgSource()
{
	this->Clear();

	delete this->destinationArray;
}

void MidiMsgSource::AddDestination(MidiMsgDestination* destination)
{
	this->destinationArray->push_back(destination);
}

void MidiMsgSource::Clear()
{
	for (MidiMsgDestination* destination : *this->destinationArray)
		delete destination;

	this->destinationArray->clear();
}

/*virtual*/ bool MidiMsgSource::Setup(Error& error)
{
	for (MidiMsgDestination* destination : *this->destinationArray)
	{
		if (!destination->Initialize(error))
		{
			error.Add("Failed to initialize MIDI message destination.");
			return false;
		}
	}

	return true;
}

/*virtual*/ bool MidiMsgSource::Shutdown(Error& error)
{
	for (MidiMsgDestination* destination : *this->destinationArray)
		destination->Finalize(error);

	return true;
}

/*virtual*/ bool MidiMsgSource::Process(Error& error)
{
	for (MidiMsgDestination* destination : *this->destinationArray)
		if (!destination->Process(error))
			return false;

	return true;
}

bool MidiMsgSource::BroadcastMidiMessage(double deltaTimeSeconds, const uint8_t* messageBuffer, uint64_t messageBufferSize, Error& error)
{
	for (MidiMsgDestination* destination : *this->destinationArray)
		if (!destination->ReceiveMessage(deltaTimeSeconds, messageBuffer, messageBufferSize, error))
			return false;

	return true;
}