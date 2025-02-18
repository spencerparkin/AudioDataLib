#include "MidiDebugSource.h"
#include "Keyboard.h"
#include "AudioDataLib/FileDatas/MidiData.h"
#include "AudioDataLib/ByteStream.h"
#include "AudioDataLib/Error.h"

using namespace AudioDataLib;

MidiDebugSource::MidiDebugSource()
{
	this->keyboard = nullptr;
}

/*virtua*/ MidiDebugSource::~MidiDebugSource()
{
	delete this->keyboard;
}

/*virtua*/ bool MidiDebugSource::Setup(AudioDataLib::Error& error)
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

	if (!MidiMsgSource::Setup(error))
		return false;

	return true;
}

/*virtua*/ bool MidiDebugSource::Shutdown(AudioDataLib::Error& error)
{
	MidiMsgSource::Shutdown(error);

	if (this->keyboard)
	{
		std::string keyboardError;
		this->keyboard->Shutdown(keyboardError);
		delete this->keyboard;
		this->keyboard = nullptr;
	}

	return true;
}

/*virtua*/ bool MidiDebugSource::Process(AudioDataLib::Error& error)
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

	if (!MidiMsgSource::Process(error))
		return false;

	return true;
}

bool MidiDebugSource::SendNoteMessage(uint32_t note, bool onOff, Error& error)
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