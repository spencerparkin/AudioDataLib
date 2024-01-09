#include "MidiTrackData.h"

using namespace AudioDataLib;

//---------------------------- MidiTrackData ----------------------------

MidiTrackData::MidiTrackData()
{
	this->midiData = nullptr;
	this->midiPlayer = nullptr;
}

/*virtual*/ MidiTrackData::~MidiTrackData()
{
	if (this->midiPlayer)
	{
		std::string error;
		this->midiPlayer->EndPlayback(error);
		delete this->midiPlayer;
	}

	MidiData::Destroy(this->midiData);
}

/*virtual*/ void MidiTrackData::Render(wxPaintDC& paintDC) const
{
	//...
}

void MidiTrackData::SetMidiData(MidiData* midiData)
{
	if(this->midiData)
		MidiData::Destroy(this->midiData);

	this->midiData = midiData;
}

/*virtual*/ bool MidiTrackData::Process(std::string& error)
{
	if (this->midiPlayer)
		this->midiPlayer->ManagePlayback();

	return true;
}

/*virtual*/ bool MidiTrackData::BeginPlayback(std::string& error)
{
	if (this->GetState() != State::HAPPY)
	{
		error = "Can't start playing track unless it's in the happy state.";
		return false;
	}

	this->midiPlayer = new MidiPlayer();
	this->midiPlayer->SetTimeSeconds(this->timeSeconds);
	this->midiPlayer->SetMidiData(this->midiData);
	if (!this->midiPlayer->BeginPlayback(error))
	{
		delete this->midiPlayer;
		this->midiPlayer = nullptr;
		return false;
	}

	return true;
}

/*virtual*/ bool MidiTrackData::StopPlayback(std::string& error)
{
	if (this->GetState() != State::PLAYING)
	{
		error = "Can't stop playback if we're not already playing.";
		return false;
	}

	bool success = this->midiPlayer->EndPlayback(error);

	delete this->midiPlayer;
	this->midiPlayer = nullptr;

	return success;
}

/*virtual*/ bool MidiTrackData::BeginRecording(std::string& error)
{
	error = "Not yet supported.";
	return false;
}

/*virtual*/ bool MidiTrackData::StopRecording(std::string& error)
{
	error = "No yet supported.";
	return false;
}

/*virtual*/ TrackData::State MidiTrackData::GetState() const
{
	if (this->midiPlayer)
		return State::PLAYING;

	return State::HAPPY;
}

//---------------------------- MidiTrackData::MidiPlayer ----------------------------

MidiTrackData::MidiPlayer::MidiPlayer()
{
	this->midiOut = nullptr;
}

/*virtual*/ MidiTrackData::MidiPlayer::~MidiPlayer()
{
	delete this->midiOut;
}

/*virtual*/ bool MidiTrackData::MidiPlayer::BeginPlayback(std::string& error)
{
	if (this->midiOut)
	{
		error = "MIDI out already initialized!";
		return false;
	}

	bool success = true;

	try
	{
		this->midiOut = new RtMidiOut();
	
		uint32_t numPorts = this->midiOut->getPortCount();
		if (numPorts == 0)
		{
			error = "No output MIDI ports detected.";
			success = false;
		}
		else
		{
			// TODO: Maybe if there is more than one port, let the user pick which one???
			this->midiOut->openPort(0);
		}

		if (!AudioDataLib::MidiPlayer::BeginPlayback(error))
			success = false;
	}
	catch (RtMidiError& midiError)
	{
		error = midiError.getMessage();
		success = false;
	}

	if (!success)
	{
		delete this->midiOut;
		this->midiOut = nullptr;
	}

	return success;
}

/*virtual*/ bool MidiTrackData::MidiPlayer::EndPlayback(std::string& error)
{
	if (!this->midiOut)
	{
		error = "MIDI out not initialized!";
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
		error = midiError.getMessage();
		success = false;
	}

	delete this->midiOut;
	this->midiOut = nullptr;

	return success;
}

/*virtual*/ void MidiTrackData::MidiPlayer::SendMessage(const uint8_t* message, uint64_t messageSize)
{
	if (this->midiOut)
	{
		//try
		//{
			this->midiOut->sendMessage(message, (size_t)messageSize);
		//}
		//catch (RtMidiError& midiError)
		//{
		//	// TODO: Handle this.
		//}
	}
}