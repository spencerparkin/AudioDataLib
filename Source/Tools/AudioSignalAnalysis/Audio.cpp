#include "Audio.h"
#include "Error.h"
#include "AudioListControl.h"
#include <wx/glcanvas.h>

using namespace AudioDataLib;

//--------------------------------- Audio ---------------------------------

Color Audio::nextColor(1.0, 0.0, 0.0, 1.0);

Audio::Audio()
{
	this->flags = 0;
	this->color = nextColor;

	nextColor.r += 0.31;
	if (nextColor.r > 1.0)
	{
		nextColor.r -= 1.0;
		nextColor.g += 0.79;
		if (nextColor.g > 1.0)
		{
			nextColor.g -= 1.0;
			nextColor.b += 0.47;
			if (nextColor.b > 1.0)
				nextColor.b -= 1.0;
		}
	}
}

/*virtual*/ Audio::~Audio()
{
}

//--------------------------------- WaveFormAudio ---------------------------------

WaveFormAudio::WaveFormAudio()
{
}

/*virtual*/ WaveFormAudio::~WaveFormAudio()
{
}

/*virtual*/ void WaveFormAudio::Render() const
{
	this->GetWaveForm();

	const std::vector<WaveForm::Sample>& sampleArray = this->waveForm->GetSampleArray();

	if ((this->GetFlags() & AUDIO_FLAG_SHOW_SEGMENTS) != 0)
	{
		Color alternatingColors[2];
		alternatingColors[0] = this->color;
		alternatingColors[1].Invert(alternatingColors[0]);
		glBegin(GL_LINES);
		for (uint64_t i = 0; i < sampleArray.size() - 1; i++)
		{
			const WaveForm::Sample& sampleA = sampleArray[i];
			const WaveForm::Sample& sampleB = sampleArray[i + 1];
			const Color* segmentColor = &alternatingColors[i % 2];
			glColor3d(segmentColor->r, segmentColor->g, segmentColor->b);
			glVertex2d(sampleA.timeSeconds, sampleA.amplitude);
			glVertex2d(sampleB.timeSeconds, sampleB.amplitude);
		}
		glEnd();
	}
	else
	{
		glBegin(GL_LINE_STRIP);
		glColor3d(this->color.r, this->color.g, this->color.b);
		for (const WaveForm::Sample& sample : sampleArray)
			glVertex2d(sample.timeSeconds, sample.amplitude);
		glEnd();
	}
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

void WaveFormAudio::SetAudioData(std::shared_ptr<AudioDataLib::AudioData>& audioData)
{
	this->audioData = audioData;
}

void WaveFormAudio::SetWaveForm(std::shared_ptr<AudioDataLib::WaveForm>& waveForm)
{
	this->waveForm = waveForm;
}

const AudioDataLib::WaveForm* WaveFormAudio::GetWaveForm() const
{
	if (!this->waveForm && this->audioData)
	{
		Error error;
		this->waveForm.reset(new WaveForm());
		this->waveForm->ConvertFromAudioBuffer(this->audioData->GetFormat(), this->audioData->GetAudioBuffer(), this->audioData->GetAudioBufferSize(), 0, error);
	}

	return this->waveForm.get();
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
	glColor3d(this->color.r, this->color.g, this->color.b);

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