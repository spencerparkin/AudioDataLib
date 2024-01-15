#include "SimpleSynth.h"
#include "AudioSink.h"
#include "MidiData.h"
#include "Error.h"

using namespace AudioDataLib;

SimpleSynth::SimpleSynth(const AudioData::Format& format) : MidiSynth(format)
{
}

/*virtual*/ SimpleSynth::~SimpleSynth()
{
}

/*virtual*/ bool SimpleSynth::GenerateAudio(Error& error)
{
	return false;
}

// TODO: All notes could work off the same oscillator mechanism so that there is no weird discontinuity in the wave-forms.
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
				// TODO: Signal a note to taper off.
			}
			else
			{
				double noteFrequency = this->MidiPitchToFrequency(pitchValue);
				double noteVolume = this->MidiVelocityToAmplitude(velocityValue);

				// TODO: Create a new note and plug it into the audio sink.
			}			

			break;
		}
		case MidiData::ChannelEvent::NOTE_OFF:
		{
			uint8_t pitchValue = channelEvent.param1;
			// TODO: Signal a note to taper off.
			
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