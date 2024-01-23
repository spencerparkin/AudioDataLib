#include "Audio.h"
#include "Error.h"
#include "AudioListControl.h"
#include <wx/glcanvas.h>

using namespace AudioDataLib;

//--------------------------------- Audio ---------------------------------

Audio::Audio()
{
	this->flags = 0;
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
	glColor3f(1.0f, 0.0f, 0.0f);

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

/*virtual*/ wxString WaveFormAudio::GetColumnInfo(long column) const
{
	switch (column)
	{
		case AUDIO_LIST_COLUMN_NAME:
		{
			return this->GetName();
		}
		case AUDIO_LIST_COLUMN_SIZE:
		{
			if (this->audioData)
				return wxString::Format("%lld bytes", this->audioData->GetAudioBufferSize());
			break;
		}
	}

	return "?";
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

//--------------------------------- FrequencyGraphAudio ---------------------------------

FrequencyGraphAudio::FrequencyGraphAudio()
{
	this->frequencyGraph = nullptr;
}

/*virtual*/ FrequencyGraphAudio::~FrequencyGraphAudio()
{
	delete this->frequencyGraph;
}

void FrequencyGraphAudio::SetFrequencyGraph(AudioDataLib::FrequencyGraph* frequencyGraph)
{
	if (this->frequencyGraph)
		delete this->frequencyGraph;

	this->frequencyGraph = frequencyGraph;
}

/*virtual*/ void FrequencyGraphAudio::Render() const
{
	glBegin(GL_LINE_STRIP);
	glColor3f(0.0f, 1.0f, 0.0f);

	const std::vector<FrequencyGraph::Plot>& plotArray = this->frequencyGraph->GetPlotArray();
	for(const FrequencyGraph::Plot& plot : plotArray)
	{
		glVertex2d(plot.frequency, plot.strength);
	}

	glEnd();
}

/*virtual*/ Box2D FrequencyGraphAudio::CalcBoundingBox() const
{
	Box2D boundingBox;

	const std::vector<FrequencyGraph::Plot>& plotArray = this->frequencyGraph->GetPlotArray();
	for (const FrequencyGraph::Plot& plot : plotArray)
	{
		boundingBox.ExpandToIncludePoint(Vector2D(plot.frequency, plot.strength));
	}

	return boundingBox;
}

/*virtual*/ wxString FrequencyGraphAudio::GetColumnInfo(long column) const
{
	switch (column)
	{
		case AUDIO_LIST_COLUMN_NAME:
		{
			return this->GetName();
		}
		case AUDIO_LIST_COLUMN_SIZE:
		{
			return wxString::Format("%d plots", uint32_t(this->frequencyGraph->GetPlotArray().size()));
		}
	}

	return "?";
}