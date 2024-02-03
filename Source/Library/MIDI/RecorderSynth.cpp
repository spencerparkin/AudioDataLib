#include "RecorderSynth.h"
#include "MidiData.h"
#include "Error.h"

using namespace AudioDataLib;

RecorderSynth::RecorderSynth() : MidiSynth(false)
{
	this->midiData = nullptr;
	this->messageDataArray = new std::vector<MessageData*>();
}

/*virtual*/ RecorderSynth::~RecorderSynth()
{
	this->Clear();

	delete this->messageDataArray;
}

void RecorderSynth::Clear()
{
	for (MessageData* messageData : *this->messageDataArray)
		delete messageData;

	this->messageDataArray->clear();
}

/*virtual*/ bool RecorderSynth::ReceiveMessage(double deltaTimeSeconds, const uint8_t* message, uint64_t messageSize, Error& error)
{
	if (!this->midiData)
	{
		error.Add("No MIDI data object configured.");
		return false;
	}

	// It's important that we are as quick as possible in this subroutine.  That's why we just
	// store the given message away for now, and then once it's time to finalize, that's when
	// we will process all the messages into the configured MIDI data object.

	MessageData* messageData = new MessageData();
	messageData->deltaTimeSeconds = deltaTimeSeconds;
	messageData->messageBuffer.resize(messageSize);
	::memcpy(messageData->messageBuffer.data(), message, messageSize);
	this->messageDataArray->push_back(messageData);

	return true;
}

/*virtual*/ bool RecorderSynth::Finalize(Error& error)
{
	// TODO: This is the trickiest part.  How do we process all the messages we've collected into
	//       our configured MIDI data object?

	return true;
}