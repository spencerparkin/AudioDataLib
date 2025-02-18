#include "AudioDataLib/MIDI/MidiPlayer.h"
#include "AudioDataLib/FileDatas/MidiData.h"
#include "AudioDataLib/Mutex.h"
#include "AudioDataLib/Timer.h"
#include "AudioDataLib/ByteStream.h"
#include "AudioDataLib/ErrorSystem.h"

using namespace AudioDataLib;

//------------------------------- MidiPlayer -------------------------------

MidiPlayer::MidiPlayer(Timer* timer)
{
	this->timer = timer;
	this->midiData = nullptr;
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

/*virtual*/ bool MidiPlayer::Setup()
{
	this->Clear();

	if (!MidiMsgSource::Setup())
		return false;

	if (!this->midiData)
	{
		ErrorSystem::Get()->Add("No MIDI data set on player.  Can't play.");
		return false;
	}

	if (this->tracksToPlaySet->size() == 0)
	{
		ErrorSystem::Get()->Add("Given tracks-to-play set is empty.");
		return false;
	}

	if (this->midiData->GetFormatType() != MidiData::FormatType::MULTI_TRACK &&
		this->midiData->GetFormatType() != MidiData::FormatType::SINGLE_TRACK)
	{
		ErrorSystem::Get()->Add(std::format("MIDI type ({}) not yet supported.", uint32_t(this->midiData->GetFormatType())));
		return false;
	}

	const MidiData::Track* infoTrack = this->midiData->GetTrack(0);
	if (!infoTrack)
	{
		ErrorSystem::Get()->Add("Could not get info track from MIDI data.");
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
	}

	if (!this->timer)
	{
		ErrorSystem::Get()->Add("No timer set.");
		return false;
	}

	this->timer->Start();
	this->timer->SetMaxDeltaTimeSeconds(2.0);

	return true;
}

/*virtual*/ bool MidiPlayer::Shutdown()
{
	this->SilenceAllChannels();

	MidiMsgSource::Shutdown();
	
	this->Clear();

	return !ErrorSystem::Get()->Errors();
}

/*virtual*/ bool MidiPlayer::Process()
{
	if (!MidiMsgSource::Process())
		return false;

	double deltaTimeSeconds = this->timer->GetDeltaTimeSeconds();

	// Synchronization between the tracks here is called into question in my mind.  Hmmmmm.
	for (TrackPlayer* trackPlayer : *this->trackPlayerArray)
		if (!trackPlayer->Advance(deltaTimeSeconds, this))
			return false;

	return true;
}

bool MidiPlayer::SilenceAllChannels()
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
			channelEvent.Encode(messageStream);
			this->BroadcastMidiMessage(0.0, messageStream.GetBuffer(), messageStream.GetSize());
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

bool MidiPlayer::TrackPlayer::Advance(double deltaTimeSeconds, MidiPlayer* midiPlayer)
{
	const MidiData::Track* track = midiPlayer->midiData->GetTrack(this->trackOffset);
	if (!track)
	{
		ErrorSystem::Get()->Add(std::format("Failed to get track {}.", this->trackOffset));
		return false;
	}

	if (midiPlayer->midiData->GetTiming().type != MidiData::Timing::Type::TICKS_PER_QUARTER_NOTE)
	{
		ErrorSystem::Get()->Add(std::format("Timing type ({}) not yet supported.", int(midiPlayer->midiData->GetTiming().type)));
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

			if (!this->ProcessEvent(event, midiPlayer))
				return false;

			this->nextTrackEventOffset++;
		}
	}

	return true;
}

bool MidiPlayer::TrackPlayer::ProcessEvent(const MidiData::Event* event, MidiPlayer* midiPlayer)
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
		WriteOnlyBufferStream bufferStream(this->messageBuffer, ADL_MIDI_MESSAGE_BUFFER_SIZE);

		if (!channelEvent->Encode(bufferStream))
			return false;

		midiPlayer->BroadcastMidiMessage(0.0, bufferStream.GetBuffer(), bufferStream.GetSize());

		if (ErrorSystem::Get()->Errors())
			return false;
	}

	return true;
}