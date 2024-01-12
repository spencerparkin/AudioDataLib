#include "MidiPlayer.h"
#include "MidiData.h"
#include "Mutex.h"
#include "ByteStream.h"

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

/*virtual*/ bool MidiPlayer::BeginPlayback(const std::set<uint32_t>& tracksToPlaySet, std::string& error)
{
	this->Clear();

	if (!this->midiData)
	{
		error = "No MIDI data set on player.  Can't play.";
		return false;
	}

	for (uint32_t trackOffset : tracksToPlaySet)
	{
		auto trackPlayer = new TrackPlayer(trackOffset);
		this->trackPlayerArray->push_back(trackPlayer);
		if (!trackPlayer->Advance(this->timeSeconds, this, false, error))
			return false;
	}

	this->lastClock = ::clock();
	return true;
}

/*virtual*/ bool MidiPlayer::EndPlayback(std::string& error)
{
	this->Clear();

	return true;
}

/*virtual*/ bool MidiPlayer::ManagePlayback(std::string& error)
{
	clock_t presentClock = ::clock();
	clock_t elapsedClock = presentClock - this->lastClock;
	double elapsedTimeSeconds = double(elapsedClock) / double(CLOCKS_PER_SEC);
	this->lastClock = presentClock;
	this->timeSeconds += elapsedTimeSeconds;

	// Synchronization between the tracks here is called into question in my mind.  Hmmmmm.
	for (TrackPlayer* trackPlayer : *this->trackPlayerArray)
		if (!trackPlayer->Advance(elapsedTimeSeconds, this, true, error))
			return false;

	return true;
}

/*virtual*/ void MidiPlayer::SendMessage(const uint8_t* message, uint64_t messageSize)
{
	// We do nothing here by default.  The user needs to override this method.
	// The override should send the given message to a MIDI device.
}

//------------------------------- MidiPlayer::TrackPlayer -------------------------------

MidiPlayer::TrackPlayer::TrackPlayer(uint32_t trackOffset)
{
	this->trackOffset = trackOffset;
	this->nextTrackEventOffset = 0;
	this->timeSinceLastEventSeconds = 0.0;
	this->currentTempo.microsecondsPerQuarterNote = 500000;
	::memset(&this->messageBuffer, 0, ADL_MIDI_MESSAGE_BUFFER_SIZE);
}

/*virtual*/ MidiPlayer::TrackPlayer::~TrackPlayer()
{
}

bool MidiPlayer::TrackPlayer::Advance(double deltaTimeSeconds, MidiPlayer* midiPlayer, bool makeSound, std::string& error)
{
	const MidiData::Track* track = midiPlayer->midiData->GetTrack(this->trackOffset);
	if (!track)
	{
		error = "Failed to get track.";
		return false;
	}

	if (midiPlayer->midiData->GetTiming().type != MidiData::Timing::Type::TICKS_PER_QUARTER_NOTE)
	{
		error = "Timing type not yet supported.";
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

bool MidiPlayer::TrackPlayer::ProcessEvent(const MidiData::Event* event, MidiPlayer* midiPlayer, bool makeSound, std::string& error)
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
			midiPlayer->mutex->Lock();

		midiPlayer->SendMessage(bufferStream.GetBuffer(), bufferStream.GetSize());

		if (midiPlayer->mutex)
			midiPlayer->mutex->Unlock();
	}

	return true;
}