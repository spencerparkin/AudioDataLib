#pragma once

#include "Common.h"
#include "MidiData.h"

#define ADL_MIDI_MESSAGE_BUFFER_SIZE		1024

namespace AudioDataLib
{
	class MidiData;
	class Mutex;
	class Error;

	// This class is meant to be used in conjection with some other library that provides
	// access to a MIDI device.  Here we provide a convenient way to feed the MIDI device
	// appropriately for real-time synthesis and audio playback of the MIDI track data.
	// That is to say, we handle the timing of when to send the MIDI messages.
	class AUDIO_DATA_LIB_API MidiPlayer
	{
	public:
		MidiPlayer(Mutex* mutex);
		virtual ~MidiPlayer();

		void SetMidiData(const MidiData* midiData) { this->midiData = midiData; }
		const MidiData* GetMidiData() const { return this->midiData; }

		void SetTimeSeconds(double timeSeconds) { this->timeSeconds = timeSeconds; }
		double GetTimeSeconds() const { return this->timeSeconds; }

		virtual bool BeginPlayback(const std::set<uint32_t>& tracksToPlaySet, Error& error);
		virtual bool EndPlayback(Error& error);

		// This gets called every iteration of the user's main program loop.  This class
		// keeps time and knows when to fire the SendMessage() method and with what data.
		// If this method is not called often enough, it can result in faulty play-back.
		virtual bool ManagePlayback(Error& error);

		// This should get implemented by a user-drived class to send the
		// given message to the MIDI device.
		virtual void SendMessage(const uint8_t* message, uint64_t messageSize);

		bool NoMoreToPlay();

	protected:
		void Clear();

		class TrackPlayer
		{
		public:
			TrackPlayer(uint32_t trackOffset, const MidiData::MetaEvent::Tempo& tempo);
			virtual ~TrackPlayer();

			bool Advance(double deltaTimeSeconds, MidiPlayer* midiPlayer, bool makeSound, Error& error);
			bool MoreToPlay(MidiPlayer* midiPlayer);

		private:
			bool ProcessEvent(const MidiData::Event* event, MidiPlayer* midiPlayer, bool makeSound, Error& error);

			uint32_t trackOffset;
			uint32_t nextTrackEventOffset;
			double timeSinceLastEventSeconds;
			MidiData::MetaEvent::Tempo currentTempo;
			uint8_t messageBuffer[ADL_MIDI_MESSAGE_BUFFER_SIZE];
		};

		double timeSeconds;
		const MidiData* midiData;
		Mutex* mutex;
		std::vector<TrackPlayer*>* trackPlayerArray;
		clock_t lastClock;
	};
}