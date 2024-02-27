#pragma once

#include "FileData.h"
#include "ChunkParser.h"

namespace AudioDataLib
{
	class AudioData;
	class WaveForm;

	/**
	 * @brief An instance of this class represents all the data you can find in a single sound-font file.
	 * 
	 * As of this writing, not _all_ the data in a given sound-font file is loaded into this class, so not
	 * all data is preserved.  Also, there is currently an assumption that a single sound-font file corresponds
	 * to a single instrument, which is not generally true of the file format.
	 * 
	 * An instance of this class is essentially just a set of audio samples, each sample consisting of one or
	 * more AudioData class instances, one for each channel of the sample.  Yes, a single instance of AudioData
	 * can have multiple channels, but the AudioData instances (which are actually LoopedAudioData instances) put
	 * into the SoundFontData instance will always be mono, and then organized by instance of the embedded AudioSample
	 * class.  Each AudioSample corresponds to a specific pitch and velocity pair, though, oddly, such information
	 * is still just specified at the LoopedAudioData level of the data hierarchy.  This follows the structure of
	 * the sound-font file format, even if it doesn't seem (to me, at least) to make the most sense.
	 */
	class AUDIO_DATA_LIB_API SoundFontData : public FileData
	{
		friend class SoundFontFormat;

	public:
		SoundFontData();
		virtual ~SoundFontData();

		virtual void DumpInfo(FILE* fp) const override;
		virtual void DumpCSV(FILE* fp) const override;
		virtual FileData* Clone() const override;

		/**
		 * Delete all AudioSample instances from the owned array of such.
		 */
		void Clear();

		class LoopedAudioData;

		/**
		 * Find the LoopedAudioData instance in this sound-font data having the given sample ID.
		 * This is mainly useful during load of a sound-font file, and may not have much practical
		 * purpose outside of that context.
		 */
		LoopedAudioData* FindLoopedAudioData(uint32_t sampleID);

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

		/**
		 * LoopedAudioData instances are just AudioData instances with the addition of information
		 * detailing how the audio sample can be played indefinitely in a looping fasion.  As of this
		 * writing, AudioDataLib does not provide a way to generate looped audio data, but relies on
		 * other software to have already created such data in a sound-font file.  In other words, we
		 * can take advantage of looped audio-data here, but we don't generate it.
		 */
		class AUDIO_DATA_LIB_API LoopedAudioData : public AudioData
		{
		public:
			LoopedAudioData();
			virtual ~LoopedAudioData();

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

			struct MidiKeyInfo
			{
				int8_t original;
				int8_t overridingRoot;
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
			uint32_t GetSampleID() const { return this->sampleID; }
			void SetSampleID(uint32_t sampleID) { this->sampleID = sampleID; }
			void SetLocation(const Location& location) { this->location = location; }
			const Location& GetLocation() const { return this->location; }
			void SetMidiKeyInfo(const MidiKeyInfo& keyInfo) { this->keyInfo = keyInfo; }
			const MidiKeyInfo& GetMidiKeyInfo() const { return this->keyInfo; }

		protected:
			std::string* name;
			MidiKeyInfo keyInfo;
			Loop loop;
			Mode mode;
			Location location;
			ChannelType channelType;
			uint32_t sampleID;
			mutable std::shared_ptr<WaveForm>* cachedWaveForm;
		};

		class AUDIO_DATA_LIB_API AudioSample
		{
			friend class SoundFontFormat;

		public:
			AudioSample();
			virtual ~AudioSample();

			/**
			 * Delete all AudioData instances owned by this AudioSample class.
			 */
			void Clear();

			uint32_t GetNumAudioDatas() const { return this->audioDataArray->size(); }
			const AudioData* GetAudioData(uint32_t i) const;
			std::shared_ptr<AudioData> GetAudioData(uint32_t i);

			/**
			 * Find and return the LoopedAudioData instance having the given channel type.
			 */
			const LoopedAudioData* FindLoopedAudioData(LoopedAudioData::ChannelType channelType) const;

			std::vector<std::shared_ptr<AudioData>>& GetAudioDataArray() { return *this->audioDataArray; }

		protected:

			/**
			 * This is an array of looped audio data, one for each channel.  The channels are
			 * kept separate up until the moment of synthesis, because each one
			 * might have its own sample-rate and looping characteristics.  Also,
			 * I'm not yet taking pan into account, but keeping it separate would
			 * be in preparation for that as well.
			 */
			std::vector<std::shared_ptr<AudioData>>* audioDataArray;
		};

		const GeneralInfo& GetGeneralInfo() const { return *this->generalInfo; }
		GeneralInfo& GetGeneralInfo() { return *this->generalInfo; }

		uint32_t GetNumAudioSamples() const { return this->audioSampleArray->size(); }
		const AudioSample* GetAudioSample(uint32_t i) const;

		const AudioSample* FindClosestAudioSample(double pitch, double volume) const;
		const AudioSample* FindRelevantAudioSample(uint16_t midiKey, uint16_t midiVelocity) const;

	private:
		GeneralInfo* generalInfo;

		// TODO: Maybe these need to be further sub-divided by instrument?
		std::vector<AudioSample*>* audioSampleArray;
	};
}