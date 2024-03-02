#pragma once

#include "AudioData.h"

namespace AudioDataLib
{
	class AudioData;
	class WaveForm;

	/**
	 * @brief An instance of this class contains just enough data to reasonably synthesize a set of instruments using MIDI messages.
	 * 
	 * What's not contained in this class is all the model and graph data specific to a particular audio engine.
	 * Rather, a minimalistic approach is taken here where we just want a simple set of samples, each detailing
	 * the associated instrument, key ranges, velocity ranges, original pitch, looping characteristics, channel,
	 * and maybe a few other things.  The SampleBasedSynth class, or any other class for that matter, can or will
	 * make up its own system of synthesis that utilizes this data.  Admittedly, some valuable data isn't captured
	 * as a consequence of this when loading an SF2 or DLS file, but the goal here isn't completeness, but that
	 * of something simple that works reasonably well, and is easy to understand.
	 */
	class AUDIO_DATA_LIB_API WaveTableData : public FileData
	{
	public:
		WaveTableData();
		virtual ~WaveTableData();

		virtual void DumpInfo(FILE* fp) const override;
		virtual void DumpCSV(FILE* fp) const override;
		virtual FileData* Clone() const override;

		void Clear();
		void Merge(const std::vector<const WaveTableData*>& waveTableDataArray);

		class AUDIO_DATA_LIB_API AudioSampleData : public AudioData
		{
		public:
			AudioSampleData();
			virtual ~AudioSampleData();

			virtual void DumpInfo(FILE* fp) const override;
			virtual FileData* Clone() const override;

			struct Loop
			{
				uint64_t startFrame;		///< At this byte offset, the embedded loop in the sample begins.
				uint64_t endFrame;			///< At this byte offset, the embedded loop in the sample ends and is meant to wrap back to the beginning.
			};

			enum ChannelType
			{
				MONO,
				LEFT_EAR,
				RIGHT_EAR
			};

			struct Location
			{
				uint16_t minKey, maxKey;	///< This is the range of MIDI key values for which this sample can be pitch-shifted to match.
				uint16_t minVel, maxVel;	///< this is the range of MIDI velocity values for which this sample can be scaled to match.

				bool Contains(uint16_t key, uint16_t vel) const;
			};

			enum Mode : uint8_t
			{
				NOT_LOOPED,					///< The sample is not meant to be looped at all.  Think of a harp pluck or a piano key strike.
				GETS_TRAPPED_IN_LOOP,		///< After playing the sound that comes before the loop, we then stay in the loop until the loop is terminated.  Think of a trumpet or clarinet (with great lung capacity!)
				UNUSED,						///< This is unused as per the sound-font file format documentation.
				EXIT_LOOP_ON_RELEASE		///< This is just like GETS_TRAPPED_IN_LOOP, except that when the loop is termianted, we're meant to exit at the end of the loop and then play the remainder of the sample that exists after the loop.
			};

			std::shared_ptr<WaveForm> GetCachedWaveForm(uint16_t channel, Error& error) const;

			void SetName(const std::string& name) { *this->name = name; }
			const std::string& GetName() const { return *this->name; }

			const Loop& GetLoop() const { return this->loop; }
			void SetLoop(const Loop& loop) { this->loop = loop; }

			Mode GetMode() const { return this->mode; }
			void SetMode(Mode mode) { this->mode = mode; }

			void SetChannelType(ChannelType channelType) { this->channelType = channelType; }
			ChannelType GetChannelType() const { return this->channelType; }

			void SetLocation(const Location& location) { this->location = location; }
			const Location& GetLocation() const { return this->location; }

		protected:
			uint8_t instrumentNumber;
			int8_t originalPitch;
			std::string* name;
			Loop loop;
			Mode mode;
			Location location;
			ChannelType channelType;
			mutable std::shared_ptr<WaveForm>* cachedWaveForm;
		};

		uint32_t GetNumAudioSamples() const { return this->audioSampleArray->size(); }
		const AudioSampleData* GetAudioSample(uint32_t i) const;

		const AudioSampleData* FindAudioSample(uint8_t instrument, uint16_t midiKey, uint16_t midiVelocity) const;

	private:

		std::vector<std::shared_ptr<AudioSampleData>>* audioSampleArray;
	};
}