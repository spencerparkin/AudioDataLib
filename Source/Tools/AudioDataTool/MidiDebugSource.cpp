#include "MidiDebugSource.h"
#include "Keyboard.h"
#include "AudioDataLib/FileDatas/MidiData.h"
#include "AudioDataLib/ByteStream.h"
#include "AudioDataLib/ErrorSystem.h"

using namespace AudioDataLib;

MidiDebugSource::MidiDebugSource()
{
	this->keyboard = nullptr;
}

/*virtua*/ MidiDebugSource::~MidiDebugSource()
{
	delete this->keyboard;
}

/*virtua*/ bool MidiDebugSource::Setup()
{
	if (this->keyboard)
	{
		ErrorSystem::Get()->Add("Keyboard already setup!");
		return false;
	}

	this->keyboard = Keyboard::Create();

	if (!this->keyboard)
	{
		ErrorSystem::Get()->Add("Failed to create keyboard interface.");
		return false;
	}

	if (!this->keyboard->Setup())
	{
		delete this->keyboard;
		this->keyboard = nullptr;
		return false;
	}

	if (!MidiMsgSource::Setup())
		return false;

	return true;
}

/*virtua*/ bool MidiDebugSource::Shutdown()
{
	MidiMsgSource::Shutdown();

	if (this->keyboard)
	{
		this->keyboard->Shutdown();
		delete this->keyboard;
		this->keyboard = nullptr;
	}

	return true;
}

/*virtua*/ bool MidiDebugSource::Process()
{
	if (this->keyboard)
	{
		if (!this->keyboard->Process())
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

					if (!this->SendNoteMessage(event.keyCode, true))
						return false;

					break;
				}
				case Keyboard::Event::Type::KEY_RELEASED:
				{
					if (!this->SendNoteMessage(event.keyCode, false))
						return false;

					break;
				}
			}
		}
	}

	if (!MidiMsgSource::Process())
		return false;

	return true;
}

bool MidiDebugSource::SendNoteMessage(uint32_t note, bool onOff)
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
	if (!channelEvent.Encode(stream))
		return false;
	
	uint32_t messageSize = (uint32_t)stream.GetSize();
	if (messageSize == 0)
	{
		ErrorSystem::Get()->Add("Message size is zero.");
		return false;
	}

	uint8_t* messageBuffer = new uint8_t[messageSize];
	stream.ReadBytesFromStream(messageBuffer, messageSize);
	this->BroadcastMidiMessage(0.0, messageBuffer, messageSize);	// TODO: Again, probably need to keep track of time between keypresses.
	delete[] messageBuffer;

	return !ErrorSystem::Get()->Errors();
}