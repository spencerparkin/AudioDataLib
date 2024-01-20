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
	if (!this->waveForm)
	{
		Error error;
		this->waveForm = new WaveForm();
		this->waveForm->ConvertFromAudioBuffer(this->audioData->GetFormat(), this->audioData->GetAudioBuffer(), this->audioData->GetAudioBufferSize(), 0, error);
	}

	glBegin(GL_LINE_STRIP);

	glColor3f(1.0f, 1.0f, 1.0f);

	const std::vector<WaveForm::Sample>& sampleArray = this->waveForm->GetSampleArray();

	for (const WaveForm::Sample& sample : sampleArray)
	{
		glVertex2d(sample.timeSeconds, sample.amplitude);
	}

	glEnd();
}