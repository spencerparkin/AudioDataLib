#include "AudioDataLib/MIDI/MidiMsgLogDestination.h"
#include "AudioDataLib/FileDatas/MidiData.h"
#include "AudioDataLib/ErrorSystem.h"

using namespace AudioDataLib;

MidiMsgLogDestination::MidiMsgLogDestination()
{
}

/*virtual*/ MidiMsgLogDestination::~MidiMsgLogDestination()
{

}

/*virtual*/ bool MidiMsgLogDestination::ReceiveMessage(double deltaTimeSeconds, const uint8_t* message, uint64_t messageSize)
{
	ReadOnlyBufferStream messageStream(message, messageSize);

	std::string timeStr = std::format("DT {:1.2f} sec: ", deltaTimeSeconds);

	MidiData::ChannelEvent channelEvent;
	if (channelEvent.Decode(messageStream))
		this->LogMessage(timeStr + channelEvent.LogMessage());
	else
	{
		messageStream.Reset();
		MidiData::MetaEvent metaEvent;
		if (metaEvent.Decode(messageStream))
			this->LogMessage(timeStr + metaEvent.LogMessage());
		else
		{
			messageStream.Reset();
			MidiData::SystemExclusiveEvent sysEvent;
			if (sysEvent.Decode(messageStream))
				this->LogMessage(timeStr + sysEvent.LogMessage());
			else
			{
				ErrorSystem::Get()->Add("Could not decode MIDI message!");
				return false;
			}
		}
	}

	ErrorSystem::Get()->Clear();
	return true;
}