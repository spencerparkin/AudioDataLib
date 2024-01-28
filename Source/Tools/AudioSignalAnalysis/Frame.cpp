#include "Frame.h"
#include "Canvas.h"
#include "FileFormat.h"
#include "ByteStream.h"
#include "Error.h"
#include "App.h"
#include "Audio.h"
#include "SoundFontData.h"
#include "AudioListControl.h"
#include <wx/aboutdlg.h>
#include <wx/sizer.h>
#include <wx/menu.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/numdlg.h>
#include <wx/splitter.h>
#include <wx/filename.h>

using namespace AudioDataLib;

Frame::Frame(const wxPoint& pos, const wxSize& size) : wxFrame(nullptr, wxID_ANY, "Audio Signal Analysis", pos, size)
{
	wxMenu* fileMenu = new wxMenu();
	fileMenu->Append(new wxMenuItem(fileMenu, ID_ImportAudio, "Import Audio", "Import audio from a selected file."));
	fileMenu->Append(new wxMenuItem(fileMenu, ID_ExportAudio, "Export Audio", "Export audio to disk."));
	fileMenu->AppendSeparator();
	fileMenu->Append(new wxMenuItem(fileMenu, ID_Clear, "Clear", "Delete all audio."));
	fileMenu->AppendSeparator();
	fileMenu->Append(new wxMenuItem(fileMenu, ID_MakeSound, "Make Sound", "Synthesize a simple sound as a sum of sine-waves with different frequencies."));
	fileMenu->AppendSeparator();
	fileMenu->Append(new wxMenuItem(fileMenu, ID_Exit, "Exit", "Go skiing."));

	wxMenu* analyzeMenu = new wxMenu();
	analyzeMenu->Append(new wxMenuItem(analyzeMenu, ID_GenerateFrequencyGraph, "Generate Frequency Graph", "Convert from the time-domain of the wave-form to the frequency domain."));

	wxMenu* helpMenu = new wxMenu();
	helpMenu->Append(new wxMenuItem(helpMenu, ID_About, "About", "Show the about-box."));

	wxMenuBar* menuBar = new wxMenuBar();
	menuBar->Append(fileMenu, "File");
	menuBar->Append(analyzeMenu, "Analyze");
	menuBar->Append(helpMenu, "Help");
	this->SetMenuBar(menuBar);

	this->Bind(wxEVT_MENU, &Frame::OnExit, this, ID_Exit);
	this->Bind(wxEVT_MENU, &Frame::OnAbout, this, ID_About);
	this->Bind(wxEVT_MENU, &Frame::OnImportAudio, this, ID_ImportAudio);
	this->Bind(wxEVT_MENU, &Frame::OnExportAudio, this, ID_ExportAudio);
	this->Bind(wxEVT_MENU, &Frame::OnClear, this, ID_Clear);
	this->Bind(wxEVT_MENU, &Frame::OnGenerateFrequencyGraph, this, ID_GenerateFrequencyGraph);
	this->Bind(wxEVT_MENU, &Frame::OnMakeSound, this, ID_MakeSound);

	this->SetStatusBar(new wxStatusBar(this));

	wxSplitterWindow* splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE | wxSP_3D);

	this->canvas = new Canvas(splitter);
	this->audioList = new AudioListControl(splitter);

	splitter->SplitVertically(this->audioList, this->canvas, 200);

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(splitter, 1, wxALL | wxGROW, 0);
	this->SetSizer(sizer);
}

/*virtual*/ Frame::~Frame()
{
}

void Frame::OnGenerateFrequencyGraph(wxCommandEvent& event)
{
	std::vector<Audio*> selectedAudioArray;
	if (!wxGetApp().GetFlaggedAudio(selectedAudioArray, AUDIO_FLAG_SELECTED))
	{
		wxMessageBox("Select some audio first.", "Error!", wxICON_ERROR | wxOK, this);
		return;
	}

	const Audio* audio = selectedAudioArray[0];
	auto waveFormAudio = dynamic_cast<const WaveFormAudio*>(audio);
	if (!audio)
	{
		wxMessageBox("Must have wave-form audio selected.", "Error!", wxICON_ERROR | wxOK, this);
		return;
	}

	const WaveForm* waveForm = waveFormAudio->GetWaveForm();
	FrequencyGraph* frequencyGraph = new FrequencyGraph();
	Error error;
	if (!frequencyGraph->FromWaveForm(*waveForm, 8192, error))
	{
		wxMessageBox(error.GetErrorMessage(), "Error!", wxICON_ERROR | wxOK, this);
		delete frequencyGraph;
		return;
	}

	FrequencyGraphAudio* frequencyAudio = new FrequencyGraphAudio();
	frequencyAudio->SetFrequencyGraph(frequencyGraph);
	frequencyAudio->SetName("Freq. Graph of " + audio->GetName());
	wxGetApp().AddAudio(frequencyAudio);
	this->Refresh();
	this->audioList->Update();
}

void Frame::OnImportAudio(wxCommandEvent& event)
{
	wxFileDialog fileDialog(this, "Locate your audio data files.", wxEmptyString, wxEmptyString, "Wave Files (*.wav)|*.wav|Sound-Font Files (*.sf2)|*.sf2", wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);
	if (wxID_OK != fileDialog.ShowModal())
		return;

	wxArrayString audioFileArray;
	fileDialog.GetPaths(audioFileArray);

	for (const wxString& audioFile : audioFileArray)
	{
		FileFormat* fileFormat = FileFormat::CreateForFile((const char*)audioFile.c_str());
		if (!fileFormat)
			continue;

		FileInputStream inputStream(audioFile.c_str());
		if (inputStream.IsOpen())
		{
			Error error;
			FileData* fileData = nullptr;
			if (!fileFormat->ReadFromStream(inputStream, fileData, error))
				wxMessageBox(error.GetErrorMessage(), "Error!", wxICON_ERROR | wxOK, this);
			else
			{
				auto audioData = dynamic_cast<AudioData*>(fileData);
				auto soundFontData = dynamic_cast<SoundFontData*>(fileData);

				if (audioData)
				{
					WaveFormAudio* audio = new WaveFormAudio();
					audio->SetAudioData(audioData);
					audio->SetName(wxFileName(audioFile).GetName());
					wxGetApp().AddAudio(audio);
				}
				else if (soundFontData)
				{
					for (uint32_t i = 0; i < soundFontData->GetNumPitchDatas(); i++)
					{
						const SoundFontData::PitchData* pitchData = soundFontData->GetPitchData(i);
						for (uint32_t j = 0; j < pitchData->GetNumLoopedAudioDatas(); j++)
						{
							const SoundFontData::LoopedAudioData* audioData = pitchData->GetLoopedAudioData(j);
							WaveFormAudio* audio = new WaveFormAudio();
							audio->SetAudioData(dynamic_cast<AudioData*>(audioData->Clone()));
							audio->SetName(wxFileName(audioFile).GetName() + wxString::Format("_%d_%s", i, audioData->GetName().c_str()));
							wxGetApp().AddAudio(audio);
						}
					}

					delete soundFontData;
				}
				else
				{
					wxMessageBox("Could not use data from file: " + audioFile, "Error!", wxICON_ERROR | wxOK, this);
					delete fileData;
				}
			}
		}

		delete fileFormat;
	}

	this->canvas->FitContent();
	this->audioList->Update();

	this->Refresh();
}

void Frame::OnExportAudio(wxCommandEvent& event)
{
}

void Frame::OnExit(wxCommandEvent& event)
{
	this->Close(true);
}

void Frame::OnMakeSound(wxCommandEvent& event)
{
	wxNumberEntryDialog numberDialogA(this, "Frequency A?", "Enter frequency in Hz.", "Frequency", 0, 0, 5000);
	if (wxID_OK != numberDialogA.ShowModal())
		return;

	wxNumberEntryDialog numberDialogB(this, "Frequency B?", "Enter frequency in Hz.", "Frequency", 0, 0, 5000);
	if (wxID_OK != numberDialogB.ShowModal())
		return;

	WaveFormAudio* audio = new WaveFormAudio();
	WaveForm* waveForm = new WaveForm();

	uint32_t numSamples = 4098;
	double frequencyAHz = double(numberDialogA.GetValue());
	double frequencyBHz = double(numberDialogB.GetValue());
	double durationSeconds = 2.0;
	double amplitudeA = 0.1;
	double amplitudeB = 0.2;

	for (uint32_t i = 0; i < numSamples; i++)
	{
		WaveForm::Sample sample;

		sample.timeSeconds = (double(i) / double(numSamples - 1)) * durationSeconds;
		sample.amplitude =
			amplitudeA * ::sin(2.0 * ADL_PI * sample.timeSeconds * frequencyAHz) +
			amplitudeB * ::sin(2.0 * ADL_PI * sample.timeSeconds * frequencyBHz);

		waveForm->AddSample(sample);
	}

	audio->SetWaveForm(waveForm);
	audio->SetFlags(audio->GetFlags() | AUDIO_FLAG_VISIBLE);
	wxGetApp().AddAudio(audio);
	this->Refresh();
	this->audioList->Update();
}

void Frame::OnClear(wxCommandEvent& event)
{
	wxGetApp().Clear();
	this->audioList->Update();
	this->Refresh();
}

void Frame::OnAbout(wxCommandEvent& event)
{
	wxAboutDialogInfo aboutDialogInfo;

	aboutDialogInfo.SetName("Audio Signal Analysis");
	aboutDialogInfo.SetDescription("Load and analyze WAV files using FFTs.");

	wxAboutBox(aboutDialogInfo);
}