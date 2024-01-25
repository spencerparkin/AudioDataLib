#include "SimpleSynth.h"
#include "AudioSink.h"
#include "OscillatorModule.h"
#include "MidiData.h"
#include "Error.h"

using namespace AudioDataLib;

SimpleSynth::SimpleSynth(bool ownsAudioStream) : MidiSynth(ownsAudioStream)
{
	this->mixerModule = new MixerModule();
}

/*virtual*/ SimpleSynth::~SimpleSynth()
{
	delete mixerModule;
}

/*virtual*/ SynthModule* SimpleSynth::GetRootModule(uint16_t channel)
{
	// TODO: Impliment stereo somehow?  For now, we output stereo in a mono form.
	//       That is, we write channel 0, but leave channel 1 silent.
	if(channel == 0)
		return this->mixerModule;

	return nullptr;
}

/*virtual*/ bool SimpleSynth::ReceiveMessage(double deltaTimeSeconds, const uint8_t* message, uint64_t messageSize, Error& error)
{
	MidiData::ChannelEvent channelEvent;
	ReadOnlyBufferStream bufferStream(message, messageSize);
	if (!channelEvent.Decode(bufferStream, error))
	{
		// This is not really an error.  I'm just not yet responding to anything except channel events.
		return true;
	}

	// Note that we don't do anything with the channel info here, but
	// in practice, we'd use this to select the instrument that will
	// make the desired sound.  Also note that this term should not be
	// confused with the same word that describes the parallel streams
	// in an overall audio stream (for mono, stereo, etc.)
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
				this->mixerModule->SetModule(uint32_t(pitchValue), nullptr);
			}
			else
			{
				double noteFrequency = this->MidiPitchToFrequency(pitchValue);
				double noteVolume = this->MidiVelocityToAmplitude(velocityValue);
				
				OscillatorModule::WaveParams waveParams;
				waveParams.waveType = OscillatorModule::WaveType::SAWTOOTH;
				waveParams.amplitude = noteVolume;
				waveParams.frequency = noteFrequency;

				auto oscillatorModule = new OscillatorModule();
				oscillatorModule->SetWaveParams(waveParams);

				this->mixerModule->SetModule(uint32_t(pitchValue), oscillatorModule);
			}			

			break;
		}
		case MidiData::ChannelEvent::NOTE_OFF:
		{
			uint8_t pitchValue = channelEvent.param1;
			
			// Note that in a more sophisticated implimentation, we would just signal some
			// module in the dependency chain to taper-off the oscillator module according
			// to some pre-configured curve.  But the simple-synth should be just that (simple)
			// so that it can serve as a proof-of-concept for the whole system.
			this->mixerModule->SetModule(uint32_t(pitchValue), nullptr);
			
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