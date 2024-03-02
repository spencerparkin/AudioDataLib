#include "WaveTableData.h"
#include "Error.h"
#include "WaveForm.h"

using namespace AudioDataLib;

WaveTableData::WaveTableData()
{
	this->audioSampleArray = new std::vector<std::shared_ptr<AudioSampleData>>();
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
}

/*virtual*/ FileData* WaveTableData::Clone() const
{
	return nullptr;
}

void WaveTableData::Clear()
{
	this->audioSampleArray->clear();
}

void WaveTableData::Merge(const std::vector<const WaveTableData*>& waveTableDataArray)
{
}

WaveTableData::AudioSampleData::AudioSampleData()
{
	this->instrumentNumber = 0;
	this->originalPitch = -1;
	this->name = new std::string();
	this->loop.startFrame = 0;
	this->loop.endFrame = 0;
	this->channelType = ChannelType::MONO;
	this->mode = Mode::GETS_TRAPPED_IN_LOOP;
	this->cachedWaveForm = nullptr;
	::memset(&this->location, 0, sizeof(location));
}

/*virtual*/ WaveTableData::AudioSampleData::~AudioSampleData()
{
	delete this->name;
}

/*virtual*/ void WaveTableData::AudioSampleData::DumpInfo(FILE* fp) const
{
}

/*virtual*/ FileData* WaveTableData::AudioSampleData::Clone() const
{
	return nullptr;
}

std::shared_ptr<WaveForm> WaveTableData::AudioSampleData::GetCachedWaveForm(uint16_t channel, Error& error) const
{
	if (!this->cachedWaveForm->get())
	{
		this->cachedWaveForm->reset(new WaveForm());

		if (!(*this->cachedWaveForm)->ConvertFromAudioBuffer(this->GetFormat(), this->GetAudioBuffer(), this->GetAudioBufferSize(), channel, error))
		{
			error.Add("Failed to convert sample audio buffer into a wave-form.");
			this->cachedWaveForm->reset();
		}
	}

	return *this->cachedWaveForm;
}

const WaveTableData::AudioSampleData* WaveTableData::GetAudioSample(uint32_t i) const
{
	return nullptr;
}

const WaveTableData::AudioSampleData* WaveTableData::FindAudioSample(uint8_t instrument, uint16_t midiKey, uint16_t midiVelocity) const
{
	// I don't expect a linear search to give us bad performance in practice here since
	// the lists will be small, but making the time-complexity of this routine faster may
	// be somethign to keep in mind.

	return nullptr;
}