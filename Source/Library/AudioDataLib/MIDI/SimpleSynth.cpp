#include "AudioDataLib/MIDI/SimpleSynth.h"
#include "AudioDataLib/AudioSink.h"
#include "AudioDataLib/SynthModules/OscillatorModule.h"
#include "AudioDataLib/FileDatas/MidiData.h"
#include "AudioDataLib/ErrorSystem.h"

using namespace AudioDataLib;

SimpleSynth::SimpleSynth()
{
	this->mixerModule = new MixerModule();
	this->noteMap = new NoteMap();
}

/*virtual*/ SimpleSynth::~SimpleSynth()
{
	delete this->mixerModule;
	delete this->noteMap;
}

/*virtual*/ SynthModule* SimpleSynth::GetRootModule(uint16_t channel)
{
	if(channel == 0)
		return this->mixerModule;

	return nullptr;
}

/*virtual*/ bool SimpleSynth::ReceiveMessage(double deltaTimeSeconds, const uint8_t* message, uint64_t messageSize)
{
	MidiData::ChannelEvent channelEvent;
	ReadOnlyBufferStream bufferStream(message, messageSize);
	if (!channelEvent.Decode(bufferStream))
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

#if 0		// TODO: Fix this up.
	switch (channelEvent.type)
	{
		case MidiData::ChannelEvent::NOTE_ON:
		{
			uint8_t pitchValue = channelEvent.param1;
			uint8_t velocityValue = channelEvent.param2;

			NoteMap::iterator iter = this->noteMap->find(pitchValue);
			if (iter != this->noteMap->end())
			{
				uint64_t moduleID = iter->second;
				this->mixerModule->RemoveModule(moduleID);
				this->noteMap->erase(iter);
			}

			if (velocityValue > 0)
			{
				double noteFrequency = this->MidiPitchToFrequency(pitchValue);
				double noteVolume = this->MidiVelocityToAmplitude(velocityValue);
				
				OscillatorModule::WaveParams waveParams;
				waveParams.waveType = OscillatorModule::WaveType::SAWTOOTH;
				waveParams.amplitude = noteVolume;
				waveParams.frequency = noteFrequency;

				auto oscillatorModule = new OscillatorModule();
				oscillatorModule->SetWaveParams(waveParams);

				uint64_t moduleID = this->mixerModule->AddModule(oscillatorModule);
				this->noteMap->insert(std::pair<uint8_t, uint64_t>(pitchValue, moduleID));
			}			

			break;
		}
		case MidiData::ChannelEvent::NOTE_OFF:
		{
			uint8_t pitchValue = channelEvent.param1;
			
			NoteMap::iterator iter = this->noteMap->find(pitchValue);
			if (iter != this->noteMap->end())
			{
				uint64_t moduleID = iter->second;
				this->mixerModule->RemoveModule(moduleID);
				this->noteMap->erase(iter);
			}
			
			break;
		}
		default:
		{
			ErrorSystem::Get()->Add(std::format("Unrecognized channel event type: {}", channelEvent.type));
			return false;
		}
	}
#endif

	return true;
}