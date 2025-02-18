#include "AudioDataLib/FileDatas/WaveTableData.h"
#include "AudioDataLib/ErrorSystem.h"
#include "AudioDataLib/MIDI/MidiSynth.h"
#include "AudioDataLib/WaveForm.h"

using namespace AudioDataLib;

//------------------------------ WaveTableData ------------------------------

WaveTableData::WaveTableData()
{
	this->audioSampleArray = new std::vector<std::shared_ptr<AudioData>>();
}

/*virtual*/ WaveTableData::~WaveTableData()
{
	delete this->audioSampleArray;
}

/*virtual*/ void WaveTableData::DumpInfo(FILE* fp) const
{
}

/*virtual*/ void WaveTableData::DumpCSV(FILE* fp) const
{
	fprintf(fp, "Sample Name, Orig. Key, Orig. Freq., Est. Freq., Min. Freq., Max. Freq., Est. Vol., Min. Vol., Max. Vol.\n");

	for (std::shared_ptr<AudioData>& audioData : *this->audioSampleArray)
	{
		AudioSampleData* sampleData = dynamic_cast<AudioSampleData*>(audioData.get());
		if (!sampleData)
			continue;

		const AudioSampleData::MetaData& metaData = sampleData->GetMetaData();
		const AudioSampleData::Range& range = sampleData->GetRange();
		
		audioData->CalcMetaData();

		double minFreq = MidiSynth::MidiPitchToFrequency(range.minKey);
		double maxFreq = MidiSynth::MidiPitchToFrequency(range.maxKey);

		double minVol = MidiSynth::MidiVelocityToAmplitude(range.minVel);
		double maxVol = MidiSynth::MidiVelocityToAmplitude(range.maxVel);

		fprintf(fp, "%s, %d, %f, %f, %f, %f, %f, %f, %f\n",
			sampleData->GetName().c_str(),
			sampleData->GetCharacter().originalPitch,
			MidiSynth::MidiPitchToFrequency(sampleData->GetCharacter().originalPitch),
			metaData.pitch, minFreq, maxFreq,
			metaData.volume, minVol, maxVol);
	}
}

/*virtual*/ FileData* WaveTableData::Clone() const
{
	return nullptr;
}

void WaveTableData::Clear()
{
	this->audioSampleArray->clear();
}

void WaveTableData::AddSample(std::shared_ptr<AudioSampleData> audioSampleData)
{
	this->audioSampleArray->push_back(audioSampleData);
}

void WaveTableData::Merge(const std::vector<const WaveTableData*>& waveTableDataArray)
{
}

const AudioData* WaveTableData::GetAudioSample(uint32_t i) const
{
	if (0 <= i && i < this->audioSampleArray->size())
		return (*this->audioSampleArray)[i].get();

	return nullptr;
}

std::shared_ptr<AudioData> WaveTableData::GetAudioData(uint32_t i) const
{
	return (*this->audioSampleArray)[i];
}

const WaveTableData::AudioSampleData* WaveTableData::FindAudioSample(uint8_t instrument, uint16_t midiKey, uint16_t midiVelocity) const
{
	// TODO: Speed this up with an index?

	for (auto audioData : *this->audioSampleArray)
	{
		AudioSampleData* audioSampleData = dynamic_cast<AudioSampleData*>(audioData.get());
		if (!audioSampleData)
			continue;

		if (audioSampleData->GetCharacter().instrument != instrument)
			continue;

		if (audioSampleData->GetRange().Contains(midiKey, midiVelocity))
			return audioSampleData;
	}

	return nullptr;
}

//------------------------------ WaveTableData::AudioSampleData ------------------------------

WaveTableData::AudioSampleData::AudioSampleData()
{
	this->character.instrument = 1;
	this->character.originalPitch = -1;
	this->character.fineTuneCents = 0;
	this->name = new std::string();
	this->loop.startFrame = 0;
	this->loop.endFrame = 0;
	this->channelType = ChannelType::MONO;
	this->mode = Mode::GETS_TRAPPED_IN_LOOP;
	this->cachedWaveForm = new std::shared_ptr<WaveForm>();
	::memset(&this->range, 0, sizeof(range));
}

/*virtual*/ WaveTableData::AudioSampleData::~AudioSampleData()
{
	delete this->name;
	delete this->cachedWaveForm;
}

/*virtual*/ void WaveTableData::AudioSampleData::DumpInfo(FILE* fp) const
{
	// TODO: Write more here.

	AudioData::DumpInfo(fp);
}

/*virtual*/ FileData* WaveTableData::AudioSampleData::Clone() const
{
	return nullptr;
}

std::shared_ptr<WaveForm> WaveTableData::AudioSampleData::GetCachedWaveForm(uint16_t channel) const
{
	if (!this->cachedWaveForm->get())
	{
		this->cachedWaveForm->reset(new WaveForm());

		if (!(*this->cachedWaveForm)->ConvertFromAudioBuffer(this->GetFormat(), this->GetAudioBuffer(), this->GetAudioBufferSize(), channel))
		{
			ErrorSystem::Get()->Add("Failed to convert sample audio buffer into a wave-form.");
			this->cachedWaveForm->reset();
		}
	}

	return *this->cachedWaveForm;
}

bool WaveTableData::AudioSampleData::Range::Contains(uint16_t key, uint16_t vel) const
{
	if (!(this->minKey <= key && key <= this->maxKey))
		return false;

	if (this->minVel == this->maxVel)
		return true;

	return this->minVel <= vel && vel <= this->maxVel;
}

//------------------------------ SoundFontData ------------------------------

SoundFontData::SoundFontData()
{
	this->generalInfo = new GeneralInfo();
}

/*virtual*/ SoundFontData::~SoundFontData()
{
	delete this->generalInfo;
}

/*virtual*/ void SoundFontData::DumpInfo(FILE* fp) const
{
	fprintf(fp, "       Bank name: %s\n", this->generalInfo->bankName.c_str());
	fprintf(fp, "       Copyright: %s\n", this->generalInfo->copyrightClaim.c_str());
	fprintf(fp, " Sound engineers: %s\n", this->generalInfo->soundEngineerNames.c_str());
	fprintf(fp, "     For product: %s\n", this->generalInfo->intendedProductName.c_str());
	fprintf(fp, "        Comments: %s\n", this->generalInfo->comments.c_str());
	fprintf(fp, "   Creation date: %s\n", this->generalInfo->creationDate.c_str());
	fprintf(fp, "            Tool: %s\n", this->generalInfo->soundFontToolRecord.c_str());
	fprintf(fp, "Wavetable engine: %s\n", this->generalInfo->waveTableSoundEngine.c_str());
	fprintf(fp, "   Wavetable ROM: %s\n", this->generalInfo->waveTableSoundDataROM.c_str());

	WaveTableData::DumpInfo(fp);
}

//------------------------------ DownloadableSoundData ------------------------------

DownloadableSoundData::DownloadableSoundData()
{
}

/*virtual*/ DownloadableSoundData::~DownloadableSoundData()
{
}

/*virtual*/ void DownloadableSoundData::DumpInfo(FILE* fp) const
{
	// TODO: Write more here.

	WaveTableData::DumpInfo(fp);
}