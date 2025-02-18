#include "AudioDataLib/MIDI/MidiMsgSource.h"
#include "AudioDataLib/MIDI/MidiMsgDestination.h"
#include "AudioDataLib/ErrorSystem.h"

using namespace AudioDataLib;

MidiMsgSource::MidiMsgSource()
{
	this->destinationArray = new std::vector<std::shared_ptr<MidiMsgDestination>>();
}

/*virtual*/ MidiMsgSource::~MidiMsgSource()
{
	delete this->destinationArray;
}

void MidiMsgSource::AddDestination(std::shared_ptr<MidiMsgDestination> destination)
{
	this->destinationArray->push_back(destination);
}

void MidiMsgSource::Clear()
{
	this->destinationArray->clear();
}

/*virtual*/ bool MidiMsgSource::Setup()
{
	for (auto& destination : *this->destinationArray)
	{
		if (!destination->Initialize())
		{
			ErrorSystem::Get()->Add("Failed to initialize MIDI message destination.");
			return false;
		}
	}

	return true;
}

/*virtual*/ bool MidiMsgSource::Shutdown()
{
	for (auto& destination : *this->destinationArray)
		destination->Finalize();

	return true;
}

/*virtual*/ bool MidiMsgSource::Process()
{
	for (auto& destination : *this->destinationArray)
		if (!destination->Process())
			return false;

	return true;
}

bool MidiMsgSource::BroadcastMidiMessage(double deltaTimeSeconds, const uint8_t* messageBuffer, uint64_t messageBufferSize)
{
	for (auto& destination : *this->destinationArray)
		if (!destination->ReceiveMessage(deltaTimeSeconds, messageBuffer, messageBufferSize))
			return false;

	return true;
}