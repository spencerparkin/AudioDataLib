#include "Audio.h"
#include "Error.h"
#include <wx/glcanvas.h>

using namespace AudioDataLib;

//--------------------------------- Audio ---------------------------------

Audio::Audio()
{
}

/*virtual*/ Audio::~Audio()
{
}

//--------------------------------- WaveFormAudio ---------------------------------

WaveFormAudio::WaveFormAudio()
{
	this->audioData = nullptr;
	this->waveForm = nullptr;
}

/*virtual*/ WaveFormAudio::~WaveFormAudio()
{
	delete this->audioData;
	delete this->waveForm;
}

/*virtual*/ void WaveFormAudio::Render() const
{
	this->GetWaveForm();

	glBegin(GL_LINE_STRIP);

	glColor3f(1.0f, 1.0f, 1.0f);

	const std::vector<WaveForm::Sample>& sampleArray = this->waveForm->GetSampleArray();

	for (const WaveForm::Sample& sample : sampleArray)
	{
		glVertex2d(sample.timeSeconds, sample.amplitude);
	}

	glEnd();
}

/*virtual*/ Box2D WaveFormAudio::CalcBoundingBox() const
{
	Box2D boundingBox;

	this->GetWaveForm();

	const std::vector<WaveForm::Sample>& sampleArray = this->waveForm->GetSampleArray();

	for (const WaveForm::Sample& sample : sampleArray)
	{
		Vector2D samplePoint(sample.timeSeconds, sample.amplitude);
		boundingBox.ExpandToIncludePoint(samplePoint);
	}

	return boundingBox;
}

void WaveFormAudio::SetAudioData(AudioDataLib::AudioData* audioData)
{
	if (this->audioData)
		delete this->audioData;

	this->audioData = audioData;
}

void WaveFormAudio::SetWaveForm(AudioDataLib::WaveForm* waveForm)
{
	if (this->waveForm)
		delete this->waveForm;
	
	this->waveForm = waveForm;
}

const AudioDataLib::WaveForm* WaveFormAudio::GetWaveForm() const
{
	if (!this->waveForm && this->audioData)
	{
		Error error;
		this->waveForm = new WaveForm();
		this->waveForm->ConvertFromAudioBuffer(this->audioData->GetFormat(), this->audioData->GetAudioBuffer(), this->audioData->GetAudioBufferSize(), 0, error);
	}

	return this->waveForm;
}