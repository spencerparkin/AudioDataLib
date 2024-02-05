#include "MidiMessageSource.h"
#include "Keyboard.h"
#include "MidiData.h"
#include "ByteStream.h"
#include "Error.h"

using namespace AudioDataLib;

//----------------------------- MidiMessageSource -----------------------------

MidiMessageSource::MidiMessageSource()
{
}

/*virtual*/ MidiMessageSource::~MidiMessageSource()
{
	this->Clear();
}

void MidiMessageSource::AddSynth(AudioDataLib::MidiSynth* synth)
{
	this->synthArray.push_back(synth);
}

void MidiMessageSource::Clear()
{
	for (MidiSynth* synth : this->synthArray)
		delete synth;

	this->synthArray.clear();
}

/*virtual*/ bool MidiMessageSource::Setup(Error& error)
{
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

/*virtual*/ bool MidiMessageSource::Shutdown(Error& error)
{
	for (MidiSynth* synth : this->synthArray)
		synth->Finalize(error);

	return true;
}

/*virtual*/ bool MidiMessageSource::Process(Error& error)
{
	for (MidiSynth* synth : this->synthArray)
		if (!synth->GenerateAudio(error))
			return false;

	return true;
}

bool MidiMessageSource::BroadcastMidiMessage(double deltaTimeSeconds, const uint8_t* messageBuffer, uint64_t messageBufferSize, Error& error)
{
	for (MidiSynth* synth : this->synthArray)
		if (!synth->ReceiveMessage(deltaTimeSeconds, messageBuffer, messageBufferSize, error))
			return false;

	return true;
}

//----------------------------- RtMidiMessageSource -----------------------------

RtMidiMessageSource::RtMidiMessageSource(const std::string& desiredPortName)
{
	this->midiIn = nullptr;
	this->desiredPortName = desiredPortName;
}

/*virtual*/ RtMidiMessageSource::~RtMidiMessageSource()
{
}

/*virtual*/ bool RtMidiMessageSource::Setup(AudioDataLib::Error& error)
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

		if(this->midiIn->isPortOpen())
			printf("Successfully opened port %d!\n", desiredPort);
	}
	catch (RtMidiError rtError)
	{
		error.Add("RtMidi error: " + rtError.getMessage());
		return false;
	}

	if (!MidiMessageSource::Setup(error))
		return false;

	return true;
}

/*virtua*/ bool RtMidiMessageSource::Shutdown(AudioDataLib::Error& error)
{
	MidiMessageSource::Shutdown(error);

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

/*virtua*/ bool RtMidiMessageSource::Process(AudioDataLib::Error& error)
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

	if (!MidiMessageSource::Process(error))
		return false;

	return true;
}

//----------------------------- KeyboardMidiMessageSource -----------------------------

KeyboardMidiMessageSource::KeyboardMidiMessageSource()
{
	this->keyboard = nullptr;
}

/*virtua*/ KeyboardMidiMessageSource::~KeyboardMidiMessageSource()
{
	delete this->keyboard;
}

/*virtua*/ bool KeyboardMidiMessageSource::Setup(AudioDataLib::Error& error)
{
	if (this->keyboard)
	{
		error.Add("Keyboard already setup!");
		return false;
	}

	this->keyboard = Keyboard::Create();

	if (!this->keyboard)
	{
		error.Add("Failed to create keyboard interface.");
		return false;
	}

	std::string keyboardError;
	if (!this->keyboard->Setup(keyboardError))
	{
		error.Add(FormatString("Failed to setup keyboard: %s", keyboardError.c_str()));
		delete this->keyboard;
		this->keyboard = nullptr;
		return false;
	}

	if (!MidiMessageSource::Setup(error))
		return false;

	return true;
}

/*virtua*/ bool KeyboardMidiMessageSource::Shutdown(AudioDataLib::Error& error)
{
	MidiMessageSource::Shutdown(error);

	if (this->keyboard)
	{
		std::string keyboardError;
		this->keyboard->Shutdown(keyboardError);
		delete this->keyboard;
		this->keyboard = nullptr;
	}

	return true;
}

/*virtua*/ bool KeyboardMidiMessageSource::Process(AudioDataLib::Error& error)
{
	if (this->keyboard)
	{
		std::string keyboardError;
		if (!this->keyboard->Process(keyboardError))
			return false;

		Keyboard::Event event;
		while (this->keyboard->GetKeyboardEvent(event))
		{
			switch (event.type)
			{
				case Keyboard::Event::Type::KEY_PRESSED:
				{
					// TODO: This won't work, because we're getting key-press repeats.  I'm giving up for now, because I just
					//       don't care enough about being able to use the computer keyboard instead of a MIDI keyboard.

					if (!this->SendNoteMessage(event.keyCode, true, error))
						return false;

					break;
				}
				case Keyboard::Event::Type::KEY_RELEASED:
				{
					if (!this->SendNoteMessage(event.keyCode, false, error))
						return false;

					break;
				}
			}
		}
	}

	if (!MidiMessageSource::Process(error))
		return false;

	return true;
}

bool KeyboardMidiMessageSource::SendNoteMessage(uint32_t note, bool onOff, Error& error)
{
	if ('a' <= note && note <= 'g')
		note = ::toupper(note);

	if (!('A' <= note && note <= 'g'))
		return true;

	uint8_t pitchValue = 0;		// TODO: Translate note into MIDI pitch value here.
	uint8_t velocityValue = 64;

	MidiData::ChannelEvent channelEvent;
	channelEvent.channel = 0;
	channelEvent.deltaTimeTicks = 0;	// TODO: Probably need to keep track of time between keypresses.
	channelEvent.type = onOff ? MidiData::ChannelEvent::Type::NOTE_ON : MidiData::ChannelEvent::Type::NOTE_OFF;
	channelEvent.param1 = pitchValue;
	channelEvent.param2 = velocityValue;

	MemoryStream stream;
	if (!channelEvent.Encode(stream, error))
		return false;
	
	uint32_t messageSize = (uint32_t)stream.GetSize();
	if (messageSize == 0)
	{
		error.Add("Message size is zero.");
		return false;
	}

	uint8_t* messageBuffer = new uint8_t[messageSize];
	stream.ReadBytesFromStream(messageBuffer, messageSize);
	this->BroadcastMidiMessage(0.0, messageBuffer, messageSize, error);	// TODO: Again, probably need to keep track of time between keypresses.
	delete[] messageBuffer;

	return !error;
}