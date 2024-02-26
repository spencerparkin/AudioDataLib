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

	class AUDIO_DATA_LIB_API MidiPlayer : public MidiMsgSource
	{
	public:
		MidiPlayer(Timer* timer, Mutex* mutex);
		virtual ~MidiPlayer();

		virtual bool Setup(Error& error) override;
		virtual bool Shutdown(Error& error) override;
		virtual bool Process(Error& error) override;

		void SetMidiData(const MidiData* midiData) { this->midiData = midiData; }
		const MidiData* GetMidiData() const { return this->midiData; }

		void SetTimeSeconds(double timeSeconds) { this->timeSeconds = timeSeconds; }
		double GetTimeSeconds() const { return this->timeSeconds; }

		bool NoMoreToPlay();
        void ConfigureToPlayAllTracks() const;
		std::set<uint32_t>& GetTracksConfiguredToPlay() { return *this->tracksToPlaySet; }

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

		double timeSeconds;
		const MidiData* midiData;
		Mutex* mutex;
		Timer* timer;
		std::vector<TrackPlayer*>* trackPlayerArray;
		std::set<uint32_t>* tracksToPlaySet;
	};
}