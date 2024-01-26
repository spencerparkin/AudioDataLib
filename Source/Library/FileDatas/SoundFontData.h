#pragma once

#include "FileData.h"
#include "ChunkParser.h"

namespace AudioDataLib
{
	class AudioData;

	class AUDIO_DATA_LIB_API SoundFontData : public FileData
	{
		friend class SoundFontFormat;

	public:
		SoundFontData();
		virtual ~SoundFontData();

		virtual void DumpInfo(FILE* fp) const override;
		virtual FileData* Clone() const override;

		void Clear();

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

			void SetName(const std::string& name) { *this->name = name; }
			const std::string& GetName() const { return *this->name; }

			struct Loop
			{
				uint64_t startFrame;
				uint64_t endFrame;
			};

			const Loop& GetLoop() const { return this->loop; }
			void SetLoop(const Loop& loop) { this->loop = loop; }

		protected:
			std::string* name;
			Loop loop;
		};

		class AUDIO_DATA_LIB_API PitchData
		{
			friend class SoundFontFormat;

		public:
			PitchData();
			virtual ~PitchData();

			void Clear();

			uint8_t GetMIDIPitch() const { return this->midiPitch; }
			void SetMIDIPitch(uint8_t midiPitch) { this->midiPitch = midiPitch; }

			uint32_t GetNumLoopedAudioDatas() const { return this->loopedAudioDataArray->size(); }
			const LoopedAudioData* GetLoopedAudioData(uint32_t i) const;

		protected:

			// This is looped audio data, one for each channel.  The channels are
			// kept separate up until the moment of synthesis, because each one
			// might have its own sample-rate and looping characteristics.
			std::vector<LoopedAudioData*>* loopedAudioDataArray;

			// This is typically zero, maybe because the application is expected
			// to deduce the general frequency of the font sample itself?
			uint8_t midiPitch;
		};

		const GeneralInfo& GetGeneralInfo() const { return *this->generalInfo; }
		GeneralInfo& GetGeneralInfo() { return *this->generalInfo; }

		uint32_t GetNumPitchDatas() const { return this->pitchDataArray->size(); }
		const PitchData* GetPitchData(uint32_t i) const;

	private:
		GeneralInfo* generalInfo;

		// TODO: Maybe these need to be further sub-divided by instrument?
		std::vector<PitchData*>* pitchDataArray;
	};
}