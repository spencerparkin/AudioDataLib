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

		class AudioSample
		{
		public:
			AudioSample();
			virtual ~AudioSample();

			AudioData* GetAudioData() { return this->audioData; }
			const AudioData* GetAudioData() const { return this->audioData; }

			void SetName(const std::string& name) { this->name = name; }
			const std::string& GetName() const { return this->name; }

			struct Loop
			{
				uint64_t startFrame;
				uint64_t endFrame;
			};

			const Loop& GetLoop() const { return this->loop; }
			void SetLoop(const Loop& loop) { this->loop = loop; }

		protected:
			AudioData* audioData;
			std::string name;
			Loop loop;
		};

		const GeneralInfo& GetGeneralInfo() const { return *this->generalInfo; }
		GeneralInfo& GetGeneralInfo() { return *this->generalInfo; }

	private:
		GeneralInfo* generalInfo;
		std::vector<AudioSample*>* audioSampleArray;
	};
}