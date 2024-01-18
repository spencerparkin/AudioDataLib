#include "RtMidiPlayer.h"
#include "Error.h"

RtMidiPlayer::RtMidiPlayer(AudioDataLib::Timer* timer) : MidiPlayer(timer, nullptr)
{
	this->midiOut = nullptr;
}

/*virtual*/ RtMidiPlayer::~RtMidiPlayer()
{
	delete this->midiOut;
}

/*virtual*/ bool RtMidiPlayer::BeginPlayback(const std::set<uint32_t>& tracksToPlaySet, AudioDataLib::Error& error)
{
	if (this->midiOut)
	{
		error.Add("MIDI out already initialized!");
		return false;
	}

	bool success = true;

	try
	{
		this->midiOut = new RtMidiOut();

		uint32_t numPorts = this->midiOut->getPortCount();
		if (numPorts == 0)
		{
			error.Add("No output MIDI ports detected.");
			success = false;
		}
		else
		{
			// TODO: Maybe if there is more than one port, let the user pick which one???
			this->midiOut->openPort(0);
		}

		if (!AudioDataLib::MidiPlayer::BeginPlayback(tracksToPlaySet, error))
			success = false;
	}
	catch (RtMidiError& midiError)
	{
		error.Add(midiError.getMessage());
		success = false;
	}

	if (!success)
	{
		delete this->midiOut;
		this->midiOut = nullptr;
	}

	return success;
}

/*virtual*/ bool RtMidiPlayer::EndPlayback(AudioDataLib::Error& error)
{
	if (!this->midiOut)
	{
		error.Add("MIDI out not initialized!");
		return false;
	}

	bool success = true;

	if (!AudioDataLib::MidiPlayer::EndPlayback(error))
		success = false;

	try
	{
		this->midiOut->closePort();
	}
	catch (RtMidiError& midiError)
	{
		error.Add(midiError.getMessage());
		success = false;
	}

	delete this->midiOut;
	this->midiOut = nullptr;

	return success;
}

/*virtual*/ bool RtMidiPlayer::ManagePlayback(AudioDataLib::Error& error)
{
	return MidiPlayer::ManagePlayback(error);
}

/*virtual*/ bool RtMidiPlayer::SendMessage(const uint8_t* message, uint64_t messageSize, AudioDataLib::Error& error)
{
	if (this->midiOut)
	{
		try
		{
			this->midiOut->sendMessage(message, (size_t)messageSize);
		}
		catch (RtMidiError& midiError)
		{
			error.Add(midiError.getMessage());
			return false;
		}
	}

	return true;
}