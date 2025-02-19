#include "Frame.h"
#include "Canvas.h"
#include "AudioDataLib/FileFormats/FileFormat.h"
#include "AudioDataLib/ByteStream.h"
#include "AudioDataLib/ErrorSystem.h"
#include "App.h"
#include "Audio.h"
#include "AudioDataLib/FileDatas/WaveTableData.h"
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
	analyzeMenu->Append(new wxMenuItem(analyzeMenu, ID_ToggleRenderStyle, "Toggle Render Style", "Render normal or render line-segments."));

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
	this->Bind(wxEVT_MENU, &Frame::OnToggleRenderStyle, this, ID_ToggleRenderStyle);
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

void Frame::OnToggleRenderStyle(wxCommandEvent& event)
{
	std::vector<Audio*> selectedAudioArray;
	if (!wxGetApp().GetFlaggedAudio(selectedAudioArray, AUDIO_FLAG_SELECTED))
	{
		wxMessageBox("Select some audio first.", "Error!", wxICON_ERROR | wxOK, this);
		return;
	}

	for (Audio* audio : selectedAudioArray)
	{
		uint32_t flags = audio->GetFlags();

		if ((flags & AUDIO_FLAG_SHOW_SEGMENTS) != 0)
			flags &= ~AUDIO_FLAG_SHOW_SEGMENTS;
		else
			flags |= AUDIO_FLAG_SHOW_SEGMENTS;

		audio->SetFlags(flags);
	}

	this->canvas->Refresh();
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
	if (!frequencyGraph->FromWaveForm(*waveForm, 32768))
	{
		wxMessageBox(ErrorSystem::Get()->GetErrorMessage(), "Error!", wxICON_ERROR | wxOK, this);
		delete frequencyGraph;
		return;
	}

	double fundamentalFreq = frequencyGraph->EstimateFundamentalFrequency();
	wxMessageBox(wxString::Format("Est. Fund. Freq. is %f Hz.", fundamentalFreq), "Frequency", wxICON_INFORMATION | wxOK, this);

	std::shared_ptr<FrequencyGraphAudio> frequencyAudio(new FrequencyGraphAudio());
	frequencyAudio->SetFrequencyGraph(frequencyGraph);
	frequencyAudio->SetName("Freq. Graph of " + audio->GetName());
	wxGetApp().AddAudio(frequencyAudio);

	FrequencyGraph* smootherFreqGraph = new FrequencyGraph();
	frequencyGraph->GenerateSmootherGraph(*smootherFreqGraph, 5.0);
	
	frequencyAudio.reset(new FrequencyGraphAudio());
	frequencyAudio->SetFrequencyGraph(smootherFreqGraph);
	frequencyAudio->SetName("Smooth Freq. Graph of " + audio->GetName());
	wxGetApp().AddAudio(frequencyAudio);

	this->Refresh();
	this->audioList->Update();
}

void Frame::OnImportAudio(wxCommandEvent& event)
{
	wxFileDialog fileDialog(this, "Locate your audio data files.", wxEmptyString, wxEmptyString, "Wave Files (*.wav)|*.wav|Aif Files (*.aif)|*.aif|Sound-Font Files (*.sf2)|*.sf2", wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);
	if (wxID_OK != fileDialog.ShowModal())
		return;

	wxArrayString audioFileArray;
	fileDialog.GetPaths(audioFileArray);

	for (const wxString& audioFile : audioFileArray)
	{
		std::shared_ptr<FileFormat> fileFormat = FileFormat::CreateForFile((const char*)audioFile.c_str());
		if (!fileFormat.get())
			continue;

		FileInputStream inputStream(audioFile.c_str());
		if (inputStream.IsOpen())
		{
			std::unique_ptr<FileData> fileData;
			if (!fileFormat->ReadFromStream(inputStream, fileData))
				wxMessageBox(ErrorSystem::Get()->GetErrorMessage(), "Error!", wxICON_ERROR | wxOK, this);
			else
			{
				auto audioData = dynamic_cast<AudioData*>(fileData.get());
				auto waveTableData = dynamic_cast<WaveTableData*>(fileData.get());

				if (audioData)
				{
					std::shared_ptr<WaveFormAudio> audio(new WaveFormAudio());
					audio->SetAudioData(fileData);
					audio->SetName(wxFileName(audioFile).GetName());
					wxGetApp().AddAudio(audio);
				}
				else if (waveTableData)
				{
					for (uint32_t i = 0; i < waveTableData->GetNumAudioSamples(); i++)
					{
						std::shared_ptr<AudioData> audioData = waveTableData->GetAudioData(i);
						auto sampleAudioData = dynamic_cast<WaveTableData::AudioSampleData*>(audioData.get());
						std::string name = sampleAudioData ? sampleAudioData->GetName() : "";
						std::shared_ptr<WaveFormAudio> audio(new WaveFormAudio());
						audio->SetAudioData(fileData);
						audio->SetName(wxFileName(audioFile).GetName() + wxString::Format("_%d_%s", i, name.c_str()));
						wxGetApp().AddAudio(audio);
					}
				}
				else
				{
					wxMessageBox("Could not use data from file: " + audioFile, "Error!", wxICON_ERROR | wxOK, this);
				}
			}
		}
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

	std::shared_ptr<WaveFormAudio> audio(new WaveFormAudio());
	std::shared_ptr<WaveForm> waveForm(new WaveForm());

	uint32_t numSamples = 16384;
	double frequencyAHz = double(numberDialogA.GetValue());
	double frequencyBHz = double(numberDialogB.GetValue());
	double durationSeconds = 6.0;
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