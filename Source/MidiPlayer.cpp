#include "MidiPlayer.h"

using namespace AudioDataLib;

MidiPlayer::MidiPlayer()
{
	this->midiData = nullptr;
	this->timeSeconds = 0.0;
}

/*virtual*/ MidiPlayer::~MidiPlayer()
{
}

/*virtual*/ bool MidiPlayer::BeginPlayback(std::string& error)
{
	return false;
}

/*virtual*/ bool MidiPlayer::EndPlayback(std::string& error)
{
	return false;
}

/*virtual*/ void MidiPlayer::Play()
{
	// TODO: Write this.
}

/*virtual*/ void MidiPlayer::SendMessage(const uint8_t* message, uint64_t messageSize)
{
	// We do nothing here by default.  The user needs to override this method.
	// The override should send the given message to a MIDI device.
}