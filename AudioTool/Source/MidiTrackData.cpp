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
		Error error;
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

/*virtual*/ bool MidiTrackData::Process(Error& error)
{
	if (this->midiPlayer)
	{
		if (!this->midiPlayer->ManagePlayback(error))
			return false;

		if (this->midiPlayer->NoMoreToPlay())
		{
			if (!this->StopPlayback(error))
				return false;
		}
	}

	return true;
}

/*virtual*/ bool MidiTrackData::BeginPlayback(Error& error)
{
	if (this->GetState() != State::HAPPY)
	{
		error.Add("Can't start playing track unless it's in the happy state.");
		return false;
	}

	std::set<uint32_t> playTrackSet;
	for (uint32_t i = 1; i < this->midiData->GetNumTracks(); i++)
		playTrackSet.insert(i);

	this->midiPlayer = new MidiPlayer();
	this->midiPlayer->SetTimeSeconds(this->timeSeconds);
	this->midiPlayer->SetMidiData(this->midiData);
	if (!this->midiPlayer->BeginPlayback(playTrackSet, error))
	{
		delete this->midiPlayer;
		this->midiPlayer = nullptr;
		return false;
	}

	return true;
}

/*virtual*/ bool MidiTrackData::StopPlayback(Error& error)
{
	if (this->GetState() != State::PLAYING)
	{
		error.Add("Can't stop playback if we're not already playing.");
		return false;
	}

	bool success = this->midiPlayer->EndPlayback(error);

	delete this->midiPlayer;
	this->midiPlayer = nullptr;

	return success;
}

/*virtual*/ bool MidiTrackData::BeginRecording(Error& error)
{
	error.Add("Not yet supported.");
	return false;
}

/*virtual*/ bool MidiTrackData::StopRecording(Error& error)
{
	error.Add("No yet supported.");
	return false;
}

/*virtual*/ TrackData::State MidiTrackData::GetState() const
{
	if (this->midiPlayer)
		return State::PLAYING;

	return State::HAPPY;
}

/*virtual*/ bool MidiTrackData::GetStatusMessage(std::string& statusMsg) const
{
	if (this->midiPlayer)
	{
		double timeSeconds = this->midiPlayer->GetTimeSeconds();
		char msgBuf[1024];
		sprintf(msgBuf, "Player at %f seconds.", timeSeconds);
		statusMsg = msgBuf;
		return true;
	}

	return false;
}

//---------------------------- MidiTrackData::MidiPlayer ----------------------------

MidiTrackData::MidiPlayer::MidiPlayer() : AudioDataLib::MidiPlayer(nullptr)
{
	this->midiOut = nullptr;
}

/*virtual*/ MidiTrackData::MidiPlayer::~MidiPlayer()
{
	delete this->midiOut;
}

/*virtual*/ bool MidiTrackData::MidiPlayer::BeginPlayback(const std::set<uint32_t>& tracksToPlaySet, Error& error)
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

/*virtual*/ bool MidiTrackData::MidiPlayer::EndPlayback(Error& error)
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

/*virtual*/ bool MidiTrackData::MidiPlayer::SendMessage(const uint8_t* message, uint64_t messageSize, Error& error)
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