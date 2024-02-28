#pragma once

#include "Common.h"
#include "MidiData.h"
#include "MidiMsgSource.h"

#define ADL_MIDI_MESSAGE_BUFFER_SIZE		1024

namespace AudioDataLib
{
	class MidiData;
	class Mutex;
	class Error;
	class Timer;

	/**
	 * @brief This class knows how to playback MIDI files.
	 * 
	 * Again, since a goal of AudioDataLib is to depend at most on the C++ standard library,
	 * we are not sending MIDI message directly to a MIDI port for sound synthesis.  Rather,
	 * we are sending them (in proper sequence and time) to any connected MidiMsgDestination
	 * classes which, in turn, may send them to a MIDI port, or be used to synthesize the
	 * MIDI messages into sound (such as is the case with any MidiSynth derivative.)
	 */
	class AUDIO_DATA_LIB_API MidiPlayer : public MidiMsgSource
	{
	public:
		/**
		 * Construct a MIDI player to use the given timer for timing.
		 * 
		 * @param[in] timer A heap allocation for a Timer class derivative.  This class takes ownership of the memory.
		 */
		MidiPlayer(Timer* timer);
		virtual ~MidiPlayer();

		virtual bool Setup(Error& error) override;
		virtual bool Shutdown(Error& error) override;
		virtual bool Process(Error& error) override;

		/**
		 * Set the MidiData class instance containing the MIDI data that will be played back by this player.
		 * Note that we do not take ownership of the memory, and we assume that the given pointer will not
		 * go stale before this class goes out of scope.
		 */
		void SetMidiData(const MidiData* midiData) { this->midiData = midiData; }

		/**
		 * Return the MidiData class instance being used for playback by this player.
		 */
		const MidiData* GetMidiData() const { return this->midiData; }

		/**
		 * At any time, this can be called to determine if the player is still playing.
		 */
		bool NoMoreToPlay();

		/**
		 * While a subset of the set of all MIDI tracks of the given MidiData can be configured
		 * to play, this method insures that all tracks are configured to play.
		 */
        void ConfigureToPlayAllTracks() const;

		/**
		 * Return a reference to the set of tracks that should play during playback.
		 * The caller can then modify this set as they see fit before playback begins.
		 * Once playback begins, this doing so does nothing.
		 */
		std::set<uint32_t>& GetTracksConfiguredToPlay() { return *this->tracksToPlaySet; }

		/**
		 * Return a pointer to the Timer instance keeping time during playback.
		 * This could be used by the caller to know where (in time) playback is currently at.
		 */
		Timer* GetTimer() { return this->timer; }

	protected:
		void Clear();
		bool SilenceAllChannels(Error& error);

		class TrackPlayer
		{
		public:
			TrackPlayer(uint32_t trackOffset, const MidiData::MetaEvent::Tempo& tempo);
			virtual ~TrackPlayer();

			bool Advance(double deltaTimeSeconds, MidiPlayer* midiPlayer, Error& error);
			bool MoreToPlay(MidiPlayer* midiPlayer);

		private:
			bool ProcessEvent(const MidiData::Event* event, MidiPlayer* midiPlayer, Error& error);

			uint32_t trackOffset;
			uint32_t nextTrackEventOffset;
			double timeSinceLastEventSeconds;
			MidiData::MetaEvent::Tempo currentTempo;
			uint8_t messageBuffer[ADL_MIDI_MESSAGE_BUFFER_SIZE];
		};

		const MidiData* midiData;
		Timer* timer;
		std::vector<TrackPlayer*>* trackPlayerArray;
		std::set<uint32_t>* tracksToPlaySet;
	};
}