#include "LogSynth.h"
#include "MidiData.h"
#include "Error.h"

using namespace AudioDataLib;

LogSynth::LogSynth()
{
}

/*virtual*/ LogSynth::~LogSynth()
{

}

/*virtual*/ bool LogSynth::ReceiveMessage(const uint8_t* message, uint64_t messageSize, Error& error)
{
	ReadOnlyBufferStream messageStream(message, messageSize);

	MidiData::ChannelEvent channelEvent;
	if (channelEvent.Decode(messageStream, error))
		this->LogMessage(channelEvent.LogMessage());
	else
	{
		messageStream.Reset();
		MidiData::MetaEvent metaEvent;
		if (metaEvent.Decode(messageStream, error))
			this->LogMessage(metaEvent.LogMessage());
		else
		{
			messageStream.Reset();
			MidiData::SystemExclusiveEvent sysEvent;
			if (sysEvent.Decode(messageStream, error))
				this->LogMessage(sysEvent.LogMessage());
			else
			{
				error.Add("Could not decode MIDI message!");
				return false;
			}
		}
	}

	error.Clear();
	return true;
}

/*virtual*/ bool LogSynth::GenerateAudio(Error& error)
{
	return true;
}