#include "SimpleSynth.h"
#include "MidiData.h"
#include "Error.h"

using namespace AudioDataLib;

SimpleSynth::SimpleSynth(const AudioData::Format& format) : MidiSynth(format)
{
}

/*virtual*/ SimpleSynth::~SimpleSynth()
{
}

/*virtual*/ bool SimpleSynth::ReceiveMessage(const uint8_t* message, uint64_t messageSize, Error& error)
{
	MidiData::ChannelEvent channelEvent;
	ReadOnlyBufferStream bufferStream(message, messageSize);
	if (!channelEvent.Decode(bufferStream, error))
	{
		// This is not really an error.  I'm just not yet responding to anything except channel events.
		return true;
	}

	uint8_t channel = channelEvent.channel;

	switch (channelEvent.type)
	{
		case MidiData::ChannelEvent::NOTE_ON:
		{
			uint8_t pitchValue = channelEvent.param1;
			uint8_t velocityValue = channelEvent.param2;

			if (velocityValue == 0)
			{
				// This is the same as a NOTE_OFF event.
			}
			else
			{
				double noteFrequency = this->MidiPitchToFrequency(pitchValue);
				double noteVolume = this->MidiVelocityToAmplitude(velocityValue);

				// TODO: Hmmmm...
			}			

			break;
		}
		case MidiData::ChannelEvent::NOTE_OFF:
		{
			uint8_t pitchValue = channelEvent.param1;

			// I'll probably ubrubtly turn the note off here, but in a real
			// synth, the note would have some sort of tapering characteristics.
			break;
		}
		default:
		{
			error.Add(FormatString("Unrecognized channel event type: %d", channelEvent.type));
			return false;
		}
	}

	return true;
}