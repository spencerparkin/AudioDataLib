#pragma once

#include "FileData.h"
#include "ChunkParser.h"

namespace AudioDataLib
{
	class AudioData;
	class WaveForm;

	class AUDIO_DATA_LIB_API SoundFontData : public FileData
	{
		friend class SoundFontFormat;

	public:
		SoundFontData();
		virtual ~SoundFontData();

		virtual void DumpInfo(FILE* fp) const override;
		virtual void DumpCSV(FILE* fp) const override;
		virtual FileData* Clone() const override;

		void Clear();

		class LoopedAudioData;

		LoopedAudioData* FindLoopedAudioData(uint32_t sampleID);

		struct VersionTag
		{
			uint16_t major;
			uint16_t minor;
		};

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

		class AUDIO_DATA_LIB_API LoopedAudioData : public AudioData
		{
		public:
			LoopedAudioData();
			virtual ~LoopedAudioData();

			virtual void DumpInfo(FILE* fp) const override;
			virtual FileData* Clone() const override;

			struct Loop
			{
				uint64_t startFrame;
				uint64_t endFrame;
			};

			enum ChannelType
			{
				MONO,
				LEFT_EAR,
				RIGHT_EAR
			};

			struct Location
			{
				uint16_t minKey, maxKey;
				uint16_t minVel, maxVel;

				bool Contains(uint16_t key, uint16_t vel) const;
			};

			enum Mode : uint8_t
			{
				NOT_LOOPED,
				GETS_TRAPPED_IN_LOOP,
				UNUSED,
				EXIT_LOOP_ON_RELEASE
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

			void Clear();

			uint32_t GetNumLoopedAudioDatas() const { return this->loopedAudioDataArray->size(); }
			const LoopedAudioData* GetLoopedAudioData(uint32_t i) const;

			const LoopedAudioData* FindLoopedAudioData(LoopedAudioData::ChannelType channelType) const;

			std::vector<LoopedAudioData*>& GeLoopedAudioDataArray() { return *this->loopedAudioDataArray; }

		protected:

			// This is looped audio data, one for each channel.  The channels are
			// kept separate up until the moment of synthesis, because each one
			// might have its own sample-rate and looping characteristics.  Also,
			// I'm not yet taking pan into account yet, but keeping it separate would
			// be in preparation for that.
			std::vector<LoopedAudioData*>* loopedAudioDataArray;
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