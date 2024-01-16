#include "MidiPlayer.h"
#include "MidiData.h"
#include "Mutex.h"
#include "ByteStream.h"
#include "Error.h"

using namespace AudioDataLib;

//------------------------------- MidiPlayer -------------------------------

MidiPlayer::MidiPlayer(Mutex* mutex)
{
	this->mutex = mutex;
	this->midiData = nullptr;
	this->timeSeconds = 0.0;
	this->trackPlayerArray = new std::vector<TrackPlayer*>();
	this->lastClock = 0;
}

/*virtual*/ MidiPlayer::~MidiPlayer()
{
	this->Clear();

	delete this->trackPlayerArray;
}

void MidiPlayer::Clear()
{
	for (TrackPlayer* trackPlayer : *this->trackPlayerArray)
		delete trackPlayer;

	this->trackPlayerArray->clear();
}

void MidiPlayer::GetSimultaneouslyPlayableTracks(std::set<uint32_t>& playableTracksSet) const
{
    playableTracksSet.clear();

    if(this->midiData->GetFormatType() == MidiData::FormatType::SINGLE_TRACK)
    {
        if(this->midiData->GetNumTracks() == 1)
            playableTracksSet.insert(0);
    }
    else if(this->midiData->GetFormatType() == MidiData::FormatType::MULTI_TRACK)
    {
        for(uint32_t i = 1; i < this->midiData->GetNumTracks(); i++)
            playableTracksSet.insert(i);
    }
}

/*virtual*/ bool MidiPlayer::BeginPlayback(const std::set<uint32_t>& tracksToPlaySet, Error& error)
{
	this->Clear();

	if (!this->midiData)
	{
		error.Add("No MIDI data set on player.  Can't play.");
		return false;
	}

	MidiData::MetaEvent::Tempo initialTempo{ 500000 };

	if (this->midiData->GetFormatType() == MidiData::FormatType::MULTI_TRACK)
	{
		const MidiData::Track* infoTrack = this->midiData->GetTrack(0);
		if (!infoTrack)
		{
			error.Add("Could not get info track for multi-track MIDI data.");
			return false;
		}

		const MidiData::MetaEvent* tempoEvent = infoTrack->FindMetaEventOfType(MidiData::MetaEvent::Type::SET_TEMPO);
		if (tempoEvent)
			initialTempo = *tempoEvent->GetData<MidiData::MetaEvent::Tempo>();
	}

	for (uint32_t trackOffset : tracksToPlaySet)
	{
		auto trackPlayer = new TrackPlayer(trackOffset, initialTempo);
		this->trackPlayerArray->push_back(trackPlayer);
		if (!trackPlayer->Advance(this->timeSeconds, this, false, error))
			return false;
	}

	this->lastClock = ::clock();
	return true;
}

/*virtual*/ bool MidiPlayer::EndPlayback(Error& error)
{
    // TODO: We should send a NOTE_OFF event for every channel.

	this->Clear();

	return true;
}

/*virtual*/ bool MidiPlayer::ManagePlayback(Error& error)
{
	clock_t presentClock = ::clock();
	clock_t elapsedClock = presentClock - this->lastClock;
	double elapsedTimeSeconds = double(elapsedClock) / double(CLOCKS_PER_SEC);
	this->lastClock = presentClock;

	// This is to prevent break-points in the debugger from messing up the playback.
	if (elapsedTimeSeconds > 2.0)
		return true;

	this->timeSeconds += elapsedTimeSeconds;

	// Synchronization between the tracks here is called into question in my mind.  Hmmmmm.
	for (TrackPlayer* trackPlayer : *this->trackPlayerArray)
		if (!trackPlayer->Advance(elapsedTimeSeconds, this, true, error))
			return false;

	return true;
}

/*virtual*/ bool MidiPlayer::SendMessage(const uint8_t* message, uint64_t messageSize, Error& error)
{
	// We do nothing here by default.  The user needs to override this method.
	// The override should send the given message to a MIDI device.

	return true;
}

bool MidiPlayer::NoMoreToPlay()
{
	for (TrackPlayer* trackPlayer : *this->trackPlayerArray)
		if (trackPlayer->MoreToPlay(this))
			return false;
	
	return true;
}

//------------------------------- MidiPlayer::TrackPlayer -------------------------------

MidiPlayer::TrackPlayer::TrackPlayer(uint32_t trackOffset, const MidiData::MetaEvent::Tempo& tempo)
{
	this->trackOffset = trackOffset;
	this->nextTrackEventOffset = 0;
	this->timeSinceLastEventSeconds = 0.0;
	this->currentTempo = tempo;
	::memset(&this->messageBuffer, 0, ADL_MIDI_MESSAGE_BUFFER_SIZE);
}

/*virtual*/ MidiPlayer::TrackPlayer::~TrackPlayer()
{
}

bool MidiPlayer::TrackPlayer::MoreToPlay(MidiPlayer* midiPlayer)
{
	const MidiData::Track* track = midiPlayer->midiData->GetTrack(this->trackOffset);
	if (!track)
		return false;

	return this->nextTrackEventOffset < track->GetEventArray().size();
}

bool MidiPlayer::TrackPlayer::Advance(double deltaTimeSeconds, MidiPlayer* midiPlayer, bool makeSound, Error& error)
{
	const MidiData::Track* track = midiPlayer->midiData->GetTrack(this->trackOffset);
	if (!track)
	{
		error.Add(FormatString("Failed to get track %d.", this->trackOffset));
		return false;
	}

	if (midiPlayer->midiData->GetTiming().type != MidiData::Timing::Type::TICKS_PER_QUARTER_NOTE)
	{
		error.Add(FormatString("Timing type (%d) not yet supported.", midiPlayer->midiData->GetTiming().type));
		return false;
	}

	double microsecondsPerTick = double(this->currentTempo.microsecondsPerQuarterNote) / double(midiPlayer->midiData->GetTiming().ticksPerQuarterNote);

	this->timeSinceLastEventSeconds += deltaTimeSeconds;

	while (true)
	{
		const MidiData::Event* event = track->GetEvent(this->nextTrackEventOffset);
		if (!event)
			break;

		constexpr double microsecondsPerSecond = 1000000.0;
		double timeBeforeNextEventSeconds = (double(event->deltaTimeTicks) * microsecondsPerTick) / microsecondsPerSecond;

		if (this->timeSinceLastEventSeconds < timeBeforeNextEventSeconds)
			break;
		else
		{
			this->timeSinceLastEventSeconds -= timeBeforeNextEventSeconds;

			if (!this->ProcessEvent(event, midiPlayer, makeSound, error))
				return false;

			this->nextTrackEventOffset++;
		}
	}

	return true;
}

bool MidiPlayer::TrackPlayer::ProcessEvent(const MidiData::Event* event, MidiPlayer* midiPlayer, bool makeSound, Error& error)
{
	const MidiData::MetaEvent* metaEvent = dynamic_cast<const MidiData::MetaEvent*>(event);
	const MidiData::ChannelEvent* channelEvent = dynamic_cast<const MidiData::ChannelEvent*>(event);

	if (metaEvent)
	{
		switch (metaEvent->type)
		{
			case MidiData::MetaEvent::Type::SET_TEMPO:
			{
				this->currentTempo = *metaEvent->GetData<MidiData::MetaEvent::Tempo>();
				break;
			}
            default:
            {
                break;
            }
		}
	}
	else if (channelEvent)
	{
		if (channelEvent->type == MidiData::ChannelEvent::Type::NOTE_ON && !makeSound)
			return true;

		WriteOnlyBufferStream bufferStream(this->messageBuffer, ADL_MIDI_MESSAGE_BUFFER_SIZE);

		if (!channelEvent->Encode(bufferStream, error))
			return false;

		if (midiPlayer->mutex)
			midiPlayer->mutex->Lock();

		midiPlayer->SendMessage(bufferStream.GetBuffer(), bufferStream.GetSize(), error);

		if (midiPlayer->mutex)
			midiPlayer->mutex->Unlock();

		if (error)
			return false;
	}

	return true;
}