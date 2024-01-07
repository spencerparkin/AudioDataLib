#include "MidiPlayer.h"

using namespace AudioDataLib;

MidiPlayer::MidiPlayer()
{
	this->midiData = nullptr;
}

/*virtual*/ MidiPlayer::~MidiPlayer()
{
}

void MidiPlayer::Play()
{
	// TODO: Write this.
}

/*virtual*/ void MidiPlayer::SendMessage(uint32_t message)
{
	// We do thing here by default.  The user needs to override this method.
	// The override should send the given message to a MIDI device.
}