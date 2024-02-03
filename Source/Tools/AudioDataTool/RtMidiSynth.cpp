#include "RtMidiSynth.h"
#include "Error.h"

using namespace AudioDataLib;

RtMidiSynth::RtMidiSynth()
{
	this->midiIn = nullptr;
}

/*virtual*/ RtMidiSynth::~RtMidiSynth()
{
	this->Clear();
}

void RtMidiSynth::AddSynth(AudioDataLib::MidiSynth* synth)
{
	this->synthArray.push_back(synth);
}

void RtMidiSynth::Clear()
{
	for (MidiSynth* synth : this->synthArray)
		delete synth;

	this->synthArray.clear();
}

bool RtMidiSynth::Setup(const std::string& desiredPortName, AudioDataLib::Error& error)
{
	if (this->midiIn)
	{
		error.Add("MIDI input already setup!");
		return false;
	}

	try
	{
		// Note that we do *not* make use of the message callback mechanism here,
		// because our MIDI-synth objects are not thread-safe, nor do I think it
		// is a good idea to try to make them thread-safe.  It's more efficient to
		// just let RtMidi provide the main thread with MIDI messages over its own
		// thread-safe queue, and to let us process those messages on the main thread.
		this->midiIn = new RtMidiIn(RtMidi::Api::UNSPECIFIED, "AudioDataTool Client");

		uint32_t portCount = this->midiIn->getPortCount();
		uint32_t desiredPort = -1;
		for (uint32_t i = 0; i < portCount; i++)
		{
			std::string portName = this->midiIn->getPortName(i);
			printf("%d: Found MIDI port: %s\n", i, portName.c_str());
			if (portName == desiredPortName)
				desiredPort = i;
		}

		if (desiredPort == -1)
		{
			error.Add(FormatString("Failed to find desired port: %s\n", desiredPortName.c_str()));
			return false;
		}

		this->midiIn->openPort(desiredPort);

		if(this->midiIn->isPortOpen())
			printf("Successfully opened port %d!\n", desiredPort);
	}
	catch (RtMidiError rtError)
	{
		error.Add("RtMidi error: " + rtError.getMessage());
		return false;
	}

	for (MidiSynth* synth : this->synthArray)
	{
		if (!synth->Initialize(error))
		{
			error.Add("Failed to initialize synth.");
			return false;
		}
	}

	return true;
}

bool RtMidiSynth::Shutdown(AudioDataLib::Error& error)
{
	for (MidiSynth* synth : this->synthArray)
		synth->Finalize(error);

	if (this->midiIn)
	{
		try
		{
			if (this->midiIn->isPortOpen())
				this->midiIn->closePort();
		}
		catch (RtMidiError rtError)
		{
			error.Add("RtMidi error: " + rtError.getMessage());
		}

		delete this->midiIn;
		this->midiIn = nullptr;
	}

	return !error;
}

bool RtMidiSynth::Process(AudioDataLib::Error& error)
{
	if (!this->midiIn)
	{
		error.Add("No MIDI input setup!");
		return false;
	}

	if (!this->midiIn->isPortOpen())
	{
		error.Add("MIDI port closed!");
		return false;
	}

	std::vector<unsigned char> message;
	double deltaTimeSeconds = this->midiIn->getMessage(&message);
	if (message.size() > 0)
		for (MidiSynth* synth : this->synthArray)
			if (!synth->ReceiveMessage(deltaTimeSeconds, (const uint8_t*)message.data(), message.size(), error))
				return false;

	for (MidiSynth* synth : this->synthArray)
		if (!synth->GenerateAudio(error))
			return false;

	return true;
}