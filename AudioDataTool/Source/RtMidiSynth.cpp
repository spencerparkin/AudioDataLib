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
		this->midiIn = new RtMidiIn(RtMidi::Api::UNSPECIFIED, "AudioDataTool Client");
		this->midiIn->setCallback(&RtMidiSynth::CallbackEntryPoint, this);

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
	}
	catch (RtMidiError rtError)
	{
		error.Add("RtMidi error: " + rtError.getMessage());
		return false;
	}

	return true;
}

bool RtMidiSynth::Shutdown(AudioDataLib::Error& error)
{
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

	return error;
}

/*static*/ void RtMidiSynth::CallbackEntryPoint(double timeStamp, std::vector<unsigned char>* message, void* userData)
{
	auto synth = reinterpret_cast<RtMidiSynth*>(userData);
	synth->Callback(timeStamp, message);
}

void RtMidiSynth::Callback(double timeStamp, std::vector<unsigned char>* message)
{
	// TODO: What about the time-stamp?
	Error error;
	for (MidiSynth* synth : this->synthArray)
		synth->ReceiveMessage((const uint8_t*)message->data(), message->size(), error);
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

	return true;
}