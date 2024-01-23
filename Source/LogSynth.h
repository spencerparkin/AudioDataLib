#pragma once

#include "MidiSynth.h"

namespace AudioDataLib
{
	// This class is used mainly for diagnostic purposes.  It does
	// nothing more than "synthesize" MIDI messages into log messages
	// that the user can print to the screen or a file.
	class AUDIO_DATA_LIB_API LogSynth : public MidiSynth
	{
	public:
		LogSynth();
		virtual ~LogSynth();

		virtual bool ReceiveMessage(double deltaTimeSeconds, const uint8_t* message, uint64_t messageSize, Error& error) override;
		virtual bool GenerateAudio(Error& error) override;

		// The user should override this to provide a place for the log output to go.
		virtual void LogMessage(const std::string& message) = 0;
	};
};