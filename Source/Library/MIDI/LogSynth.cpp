#include "LogSynth.h"
#include "MidiData.h"
#include "Error.h"

using namespace AudioDataLib;

LogSynth::LogSynth() : MidiSynth(false)
{
}

/*virtual*/ LogSynth::~LogSynth()
{

}

/*virtual*/ bool LogSynth::ReceiveMessage(double deltaTimeSeconds, const uint8_t* message, uint64_t messageSize, Error& error)
{
	ReadOnlyBufferStream messageStream(message, messageSize);

	std::string timeStr = FormatString("DT %1.2f sec: ", deltaTimeSeconds);

	MidiData::ChannelEvent channelEvent;
	if (channelEvent.Decode(messageStream, error))
		this->LogMessage(timeStr + channelEvent.LogMessage());
	else
	{
		messageStream.Reset();
		MidiData::MetaEvent metaEvent;
		if (metaEvent.Decode(messageStream, error))
			this->LogMessage(timeStr + metaEvent.LogMessage());
		else
		{
			messageStream.Reset();
			MidiData::SystemExclusiveEvent sysEvent;
			if (sysEvent.Decode(messageStream, error))
				this->LogMessage(timeStr + sysEvent.LogMessage());
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