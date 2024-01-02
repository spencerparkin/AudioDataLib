#include "WaveForm.h"

using namespace AudioDataLib;

WaveForm::WaveForm()
{
}

/*virtual*/ WaveForm::~WaveForm()
{
}

void WaveForm::ConvertFromAudioBuffer(const AudioData::Format& format, const uint8_t* audioBuffer, uint64_t audioBufferSize, uint16_t channel)
{
}

void WaveForm::ConvertToAudioBuffer(const AudioData::Format& format, uint8_t* audioBuffer, uint64_t audioBufferSize, uint16_t channel) const
{
}

double WaveForm::EvaluateAt(double timeSeconds) const
{
	return 0.0;
}

void WaveForm::GenerateIndex()
{
}

void WaveForm::SumTogether(const std::list<WaveForm*>& waveFormList)
{

}