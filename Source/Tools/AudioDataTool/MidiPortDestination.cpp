#include "MidiPortDestination.h"
#include "AudioDataLib/ErrorSystem.h"

using namespace AudioDataLib;

MidiPortDestination::MidiPortDestination()
{
	this->midiOut = nullptr;
}

/*virtual*/ MidiPortDestination::~MidiPortDestination()
{
	delete this->midiOut;
}

/*virtual*/ bool MidiPortDestination::Initialize()
{
	if (this->midiOut)
	{
		ErrorSystem::Get()->Add("MIDI out already initialized!");
		return false;
	}

	bool success = true;

	try
	{
		this->midiOut = new RtMidiOut();

		uint32_t numPorts = this->midiOut->getPortCount();
		if (numPorts == 0)
		{
			ErrorSystem::Get()->Add("No output MIDI ports detected.");
			success = false;
		}
		else
		{
			// TODO: Maybe if there is more than one port, let the user pick which one???
			this->midiOut->openPort(0);
		}
	}
	catch (RtMidiError& midiError)
	{
		ErrorSystem::Get()->Add(midiError.getMessage());
		success = false;
	}

	if (!success)
	{
		delete this->midiOut;
		this->midiOut = nullptr;
	}

	return success;
}

/*virtual*/ bool MidiPortDestination::Finalize()
{
	if (!this->midiOut)
	{
		ErrorSystem::Get()->Add("MIDI out not initialized!");
		return false;
	}

	bool success = true;

	try
	{
		this->midiOut->closePort();
	}
	catch (RtMidiError& midiError)
	{
		ErrorSystem::Get()->Add(midiError.getMessage());
		success = false;
	}

	delete this->midiOut;
	this->midiOut = nullptr;

	return success;
}

/*virtual*/ bool MidiPortDestination::ReceiveMessage(double deltaTimeSeconds, const uint8_t* message, uint64_t messageSize)
{
	if (this->midiOut)
	{
		try
		{
			this->midiOut->sendMessage(message, (size_t)messageSize);
		}
		catch (RtMidiError& midiError)
		{
			ErrorSystem::Get()->Add(midiError.getMessage());
			return false;
		}
	}

	return true;
}