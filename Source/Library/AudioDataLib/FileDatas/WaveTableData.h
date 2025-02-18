#pragma once

#include "AudioDataLib/FileDatas/AudioData.h"

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

		class AudioSampleData;

		void Clear();
		void AddSample(std::shared_ptr<AudioSampleData> audioSampleData);
		void Merge(const std::vector<const WaveTableData*>& waveTableDataArray);

		/**
		 * @brief This is AudioData with extra information needed by a synthesizer.
		 * 
		 * Note that the raw audio data stored here should be mono, and then there is a ChannelType
		 * member that is used to indicate the intended purpose of the sample.
		 */
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
				MONO,						///< The audio sample is left/right ignostic.
				LEFT_EAR,					///< The audio sample is meant for the left ear.
				RIGHT_EAR					///< The audio sample is meant for the right ear.
			};

			struct Range
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

			struct Character
			{
				uint8_t instrument;			///< This is a number that identifies the instrument.  A standard mapping is defined, but is not necessary.  Channels are mapped to instrument numbers.
				int8_t originalPitch;		///< This is the MIDI key whose pitch represents that of the original sample without any pitch shifting.
				int16_t fineTuneCents;		///< This is a pitch correction of the recorded sample.
			};

			std::shared_ptr<WaveForm> GetCachedWaveForm(uint16_t channel) const;

			void SetName(const std::string& name) { *this->name = name; }
			const std::string& GetName() const { return *this->name; }

			const Loop& GetLoop() const { return this->loop; }
			void SetLoop(const Loop& loop) { this->loop = loop; }

			Mode GetMode() const { return this->mode; }
			void SetMode(Mode mode) { this->mode = mode; }

			void SetChannelType(ChannelType channelType) { this->channelType = channelType; }
			ChannelType GetChannelType() const { return this->channelType; }

			void SetRange(const Range& range) { this->range = range; }
			const Range& GetRange() const { return this->range; }

			void SetCharacter(const Character& character) { this->character = character; }
			const Character& GetCharacter() const { return this->character; }
			Character& GetCharacter() { return this->character; }

		protected:
			std::string* name;
			Character character;
			Loop loop;
			Mode mode;
			Range range;
			ChannelType channelType;
			// TODO: Add envelope for ADSR?
			mutable std::shared_ptr<WaveForm>* cachedWaveForm;
		};

		uint32_t GetNumAudioSamples() const { return this->audioSampleArray->size(); }
		const AudioData* GetAudioSample(uint32_t i) const;
		std::shared_ptr<AudioData> GetAudioData(uint32_t i) const;

		const AudioSampleData* FindAudioSample(uint8_t instrument, uint16_t midiKey, uint16_t midiVelocity) const;

	private:

		std::vector<std::shared_ptr<AudioData>>* audioSampleArray;
	};

	/**
	 * @brief This is the data you get when you load a sound-font file.
	 */
	class AUDIO_DATA_LIB_API SoundFontData : public WaveTableData
	{
	public:
		SoundFontData();
		virtual ~SoundFontData();

		virtual void DumpInfo(FILE* fp) const override;

		struct VersionTag
		{
			uint16_t major;
			uint16_t minor;
		};

		/**
		 * This structure contains a bunch of meta-data about the sound-font.
		 */
		struct GeneralInfo
		{
			GeneralInfo()
			{
				::memset(&this->versionTag, 0, sizeof(VersionTag));
				::memset(&this->waveTableROMVersion, 0, sizeof(VersionTag));
			}

			VersionTag versionTag;
			VersionTag waveTableROMVersion;
			std::string waveTableSoundEngine;
			std::string waveTableSoundDataROM;
			std::string bankName;
			std::string creationDate;
			std::string soundEngineerNames;
			std::string intendedProductName;
			std::string copyrightClaim;
			std::string comments;
			std::string soundFontToolRecord;
		};

		const GeneralInfo& GetGeneralInfo() const { return *this->generalInfo; }
		GeneralInfo& GetGeneralInfo() { return *this->generalInfo; }

	private:
		GeneralInfo* generalInfo;
	};

	/**
	 * @brief This is the data you get when you load a DSL file.
	 */
	class DownloadableSoundData : public WaveTableData
	{
	public:
		DownloadableSoundData();
		virtual ~DownloadableSoundData();

		virtual void DumpInfo(FILE* fp) const override;

		// TODO: Add some meta-data here.
	};
}