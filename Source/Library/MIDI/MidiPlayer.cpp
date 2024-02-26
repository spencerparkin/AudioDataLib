#include "MidiPlayer.h"
#include "MidiData.h"
#include "Mutex.h"
#include "Timer.h"
#include "ByteStream.h"
#include "Error.h"

using namespace AudioDataLib;

//------------------------------- MidiPlayer -------------------------------

MidiPlayer::MidiPlayer(Timer* timer, Mutex* mutex)
{
	this->timer = timer;
	this->mutex = mutex;
	this->midiData = nullptr;
	this->timeSeconds = 0.0;
	this->trackPlayerArray = new std::vector<TrackPlayer*>();
	this->tracksToPlaySet = new std::set<uint32_t>();
}

/*virtual*/ MidiPlayer::~MidiPlayer()
{
	this->Clear();

	delete this->trackPlayerArray;
	delete this->tracksToPlaySet;
}

void MidiPlayer::Clear()
{
	for (TrackPlayer* trackPlayer : *this->trackPlayerArray)
		delete trackPlayer;

	this->trackPlayerArray->clear();
}

void MidiPlayer::ConfigureToPlayAllTracks() const
{
    this->tracksToPlaySet->clear();

    if(this->midiData->GetFormatType() == MidiData::FormatType::SINGLE_TRACK)
    {
        if(this->midiData->GetNumTracks() == 1)
			this->tracksToPlaySet->insert(0);
    }
    else if(this->midiData->GetFormatType() == MidiData::FormatType::MULTI_TRACK)
    {
        for(uint32_t i = 1; i < this->midiData->GetNumTracks(); i++)
			this->tracksToPlaySet->insert(i);
    }
}

/*virtual*/ bool MidiPlayer::Setup(Error& error)
{
	this->Clear();

	if (!MidiMsgSource::Setup(error))
		return false;

	if (!this->midiData)
	{
		error.Add("No MIDI data set on player.  Can't play.");
		return false;
	}

	if (this->tracksToPlaySet->size() == 0)
	{
		error.Add("Given tracks-to-play set is empty.");
		return false;
	}

	if (this->midiData->GetFormatType() != MidiData::FormatType::MULTI_TRACK &&
		this->midiData->GetFormatType() != MidiData::FormatType::SINGLE_TRACK)
	{
		error.Add(FormatString("MIDI type (%d) not yet supported.", uint32_t(this->midiData->GetFormatType())));
		return false;
	}

	const MidiData::Track* infoTrack = this->midiData->GetTrack(0);
	if (!infoTrack)
	{
		error.Add("Could not get info track from MIDI data.");
		return false;
	}

	MidiData::MetaEvent::Tempo initialTempo{ 500000 };
	const MidiData::MetaEvent* tempoEvent = infoTrack->FindMetaEventOfType(MidiData::MetaEvent::Type::SET_TEMPO);
	if (tempoEvent)
		initialTempo = *tempoEvent->GetData<MidiData::MetaEvent::Tempo>();

	for (uint32_t trackOffset : *this->tracksToPlaySet)
	{
		auto trackPlayer = new TrackPlayer(trackOffset, initialTempo);
		this->trackPlayerArray->push_back(trackPlayer);
		if (!trackPlayer->Advance(this->timeSeconds, this, false, error))
			return false;
	}

	if (!this->timer)
	{
		error.Add("No timer set.");
		return false;
	}

	this->timer->Start();
	this->timer->SetMaxDeltaTimeSeconds(2.0);

	return true;
}

/*virtual*/ bool MidiPlayer::Shutdown(Error& error)
{
	this->SilenceAllChannels(error);

	MidiMsgSource::Shutdown(error);
	
	this->Clear();

	return !error;
}

/*virtual*/ bool MidiPlayer::Process(Error& error)
{
	if (!MidiMsgSource::Process(error))
		return false;

	double deltaTimeSeconds = this->timer->GetDeltaTimeSeconds();

	this->timeSeconds += deltaTimeSeconds;

	// Synchronization between the tracks here is called into question in my mind.  Hmmmmm.
	for (TrackPlayer* trackPlayer : *this->trackPlayerArray)
		if (!trackPlayer->Advance(deltaTimeSeconds, this, true, error))
			return false;

	return true;
}

bool MidiPlayer::SilenceAllChannels(Error& error)
{
	for (uint8_t channel = 0; channel < 16; channel++)
	{
		MidiData::ChannelEvent channelEvent;
		channelEvent.channel = channel;
		channelEvent.type = MidiData::ChannelEvent::NOTE_OFF;
		channelEvent.param1 = 0;
		channelEvent.param2 = 0;

		for (uint8_t pitchValue = 0; pitchValue <= 127; pitchValue++)
		{
			channelEvent.param1 = pitchValue;
			uint8_t messageBuffer[128];
			WriteOnlyBufferStream messageStream(messageBuffer, sizeof(messageBuffer));
			channelEvent.Encode(messageStream, error);
			this->BroadcastMidiMessage(0.0, messageStream.GetBuffer(), messageStream.GetSize(), error);
		}
	}

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
			midiPlayer->mutex->Lock();	// TODO: After the big refactor, this mutex may be in the wrong place!

		midiPlayer->BroadcastMidiMessage(0.0, bufferStream.GetBuffer(), bufferStream.GetSize(), error);

		if (midiPlayer->mutex)
			midiPlayer->mutex->Unlock();

		if (error)
			return false;
	}

	return true;
}