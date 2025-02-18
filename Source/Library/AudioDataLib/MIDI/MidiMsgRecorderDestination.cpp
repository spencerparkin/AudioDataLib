#include "AudioDataLib/MIDI/MidiMsgRecorderDestination.h"
#include "AudioDataLib/FileDatas/MidiData.h"
#include "AudioDataLib/ByteStream.h"
#include "AudioDataLib/Error.h"

using namespace AudioDataLib;

MidiMsgRecorderDestination::MidiMsgRecorderDestination()
{
	this->midiData = nullptr;
	this->messageDataArray = new std::vector<MessageData*>();
}

/*virtual*/ MidiMsgRecorderDestination::~MidiMsgRecorderDestination()
{
	this->Clear();

	delete this->messageDataArray;
}

void MidiMsgRecorderDestination::Clear()
{
	for (MessageData* messageData : *this->messageDataArray)
		delete messageData;

	this->messageDataArray->clear();
}

/*virtual*/ bool MidiMsgRecorderDestination::ReceiveMessage(double deltaTimeSeconds, const uint8_t* message, uint64_t messageSize, Error& error)
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

/*virtual*/ bool MidiMsgRecorderDestination::Finalize(Error& error)
{
	// TODO: If multiple instruments/channels are being used in the messages, then we can't do single-track;
	//       we must do multi-track.

	if (!this->midiData)
	{
		error.Add("No MIDI data object with which to populate.");
		return false;
	}

	if (this->messageDataArray->size() == 0)
	{
		error.Add("MIDI message array is empty.  Can't populate MIDI data object without a non-empty array.");
		return false;
	}

	this->midiData->Clear();
	this->midiData->SetFormatType(MidiData::FormatType::SINGLE_TRACK);

	MidiData::Timing timing;
	timing.type = MidiData::Timing::Type::TICKS_PER_QUARTER_NOTE;
	timing.ticksPerQuarterNote = 384;		// TODO: Any method we should use for choosing this?

	this->midiData->SetTiming(timing);

	auto track = new MidiData::Track();
	this->midiData->AddTrack(track);
	
	time_t timeNow = time(nullptr);
	tm* localTime = localtime(&timeNow);

	char buffer[512];
	::strftime(buffer, sizeof(buffer), "Track recorded by AudioDataLib on %B %d, %Y @ %T.", localTime);

	auto text = new MidiData::MetaEvent::Text();
	text->buffer = new char[::strlen(buffer) + 1];
	::strcpy(text->buffer, buffer);

	auto metaEvent = new MidiData::MetaEvent();
	metaEvent->type = MidiData::MetaEvent::TRACK_NAME;
	metaEvent->data = text;
	metaEvent->deltaTimeTicks = 0;
	track->AddEvent(metaEvent);

	auto tempo = new MidiData::MetaEvent::Tempo{ 500000 };	// TODO: Any method we should use for choosing this?

	metaEvent = new MidiData::MetaEvent();
	metaEvent->type = MidiData::MetaEvent::SET_TEMPO;
	metaEvent->data = tempo;
	track->AddEvent(metaEvent);

	double ticksPerMicrosecond = double(timing.ticksPerQuarterNote) / double(tempo->microsecondsPerQuarterNote);

	for (const MessageData* messageData : *this->messageDataArray)
	{
		ReadOnlyBufferStream bufferStream(messageData->messageBuffer.data(), messageData->messageBuffer.size());

		MidiData::Event* event = nullptr;

		auto channelEvent = new MidiData::ChannelEvent();
		if (channelEvent->Decode(bufferStream, error))
			event = channelEvent;
		else
		{
			error.Clear();
			delete channelEvent;
			metaEvent = new MidiData::MetaEvent();
			bufferStream.Reset();
			if (metaEvent->Decode(bufferStream, error))
				event = metaEvent;
			else
			{
				error.Clear();
				delete metaEvent;
				auto sysEvent = new MidiData::SystemExclusiveEvent();
				bufferStream.Reset();
				if (sysEvent->Decode(bufferStream, error))
					event = sysEvent;
				else
				{
					error.Clear();
					delete sysEvent;
				}
			}
		}

		if (!event)
		{
			error.Add("Failed to decode event!");
			return false;
		}

		double deltaTimeMicroseconds = messageData->deltaTimeSeconds * 1000000.0;
		event->deltaTimeTicks = uint64_t(deltaTimeMicroseconds * ticksPerMicrosecond);

		track->AddEvent(event);
	}

	metaEvent = new MidiData::MetaEvent();
	metaEvent->type = MidiData::MetaEvent::Type::END_OF_TRACK;
	metaEvent->data = nullptr;
	track->AddEvent(metaEvent);

	printf("Populated track with %d events.\n", (uint32_t)track->GetEventArray().size());
	return true;
}