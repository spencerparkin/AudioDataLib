#include "MidiPortSource.h"
#include "ByteStream.h"
#include "Error.h"

using namespace AudioDataLib;

MidiPortSource::MidiPortSource(const std::string& desiredPortName)
{
	this->midiIn = nullptr;
	this->desiredPortName = desiredPortName;
}

/*virtual*/ MidiPortSource::~MidiPortSource()
{
}

/*virtual*/ bool MidiPortSource::Setup(AudioDataLib::Error& error)
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
			if (portName == this->desiredPortName)
				desiredPort = i;
		}

		if (desiredPort == -1)
		{
			error.Add(FormatString("Failed to find desired port: %s\n", this->desiredPortName.c_str()));
			return false;
		}

		this->midiIn->openPort(desiredPort);

		if (this->midiIn->isPortOpen())
			printf("Successfully opened port %d!\n", desiredPort);
	}
	catch (RtMidiError rtError)
	{
		error.Add("RtMidi error: " + rtError.getMessage());
		return false;
	}

	if (!MidiMsgSource::Setup(error))
		return false;

	return true;
}

/*virtua*/ bool MidiPortSource::Shutdown(AudioDataLib::Error& error)
{
	MidiMsgSource::Shutdown(error);

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

/*virtua*/ bool MidiPortSource::Process(AudioDataLib::Error& error)
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
		if (!this->BroadcastMidiMessage(deltaTimeSeconds, (const uint8_t*)message.data(), message.size(), error))
			return false;

	if (!MidiMsgSource::Process(error))
		return false;

	return true;
}