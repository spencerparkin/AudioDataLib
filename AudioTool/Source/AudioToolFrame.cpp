#include "AudioToolFrame.h"
#include "AudioTrackControl.h"
#include "AudioFileFormat.h"
#include "ByteStream.h"
#include "AudioToolApp.h"
#include <wx/aboutdlg.h>
#include <wx/menu.h>
#include <wx/toolbar.h>
#include <wx/panel.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>

using namespace AudioDataLib;

AudioToolFrame::AudioToolFrame(const wxPoint& pos, const wxSize& size) : wxFrame(nullptr, wxID_ANY, "Audio Tool", pos, size)
{
	wxMenu* fileMenu = new wxMenu();
	fileMenu->Append(new wxMenuItem(fileMenu, ID_ImportAudio, "Import Audio", "Import audio from a file on disk."));
	fileMenu->Append(new wxMenuItem(fileMenu, ID_ExportAudio, "Export Audio", "Export audio to a file on disk."));
	fileMenu->AppendSeparator();
	fileMenu->Append(new wxMenuItem(fileMenu, ID_ClearAll, "Clear All", "Remove all current tracks."));
	fileMenu->AppendSeparator();
	fileMenu->Append(new wxMenuItem(fileMenu, ID_Exit, "Exit", "Go outside for a change."));

	wxMenu* helpMenu = new wxMenu();
	helpMenu->Append(new wxMenuItem(helpMenu, ID_About, "About", "Show the about box."));

	wxMenuBar* menuBar = new wxMenuBar();
	menuBar->Append(fileMenu, "File");
	menuBar->Append(helpMenu, "Help");
	this->SetMenuBar(menuBar);

	this->SetStatusBar(new wxStatusBar(this));

	this->Bind(wxEVT_MENU, &AudioToolFrame::OnImportAudio, this, ID_ImportAudio);
	this->Bind(wxEVT_MENU, &AudioToolFrame::OnExportAudio, this, ID_ExportAudio);
	this->Bind(wxEVT_MENU, &AudioToolFrame::OnClearAll, this, ID_ClearAll);
	this->Bind(wxEVT_MENU, &AudioToolFrame::OnSkip, this, ID_SkipBackward);
	this->Bind(wxEVT_MENU, &AudioToolFrame::OnSkip, this, ID_SkipForward);
	this->Bind(wxEVT_MENU, &AudioToolFrame::OnStop, this, ID_Stop);
	this->Bind(wxEVT_MENU, &AudioToolFrame::OnPlay, this, ID_Play);
	this->Bind(wxEVT_MENU, &AudioToolFrame::OnPause, this, ID_Pause);
	this->Bind(wxEVT_MENU, &AudioToolFrame::OnRecord, this, ID_Record);
	this->Bind(wxEVT_MENU, &AudioToolFrame::OnAbout, this, ID_About);
	this->Bind(wxEVT_MENU, &AudioToolFrame::OnExit, this, ID_Exit);

	wxBitmap skipBackwardBitmap, skipForwardBitmap;
	wxBitmap stopBitmap, playBitmap, pauseBitmap, recordBitmap;

	skipBackwardBitmap.LoadFile(wxGetCwd() + "/Images/skip_backward.png", wxBITMAP_TYPE_PNG);
	skipForwardBitmap.LoadFile(wxGetCwd() + "/Images/skip_forward.png", wxBITMAP_TYPE_PNG);
	stopBitmap.LoadFile(wxGetCwd() + "/Images/stop.png", wxBITMAP_TYPE_PNG);
	playBitmap.LoadFile(wxGetCwd() + "/Images/play.png", wxBITMAP_TYPE_PNG);
	pauseBitmap.LoadFile(wxGetCwd() + "/Images/pause.png", wxBITMAP_TYPE_PNG);
	recordBitmap.LoadFile(wxGetCwd() + "/Images/record.png", wxBITMAP_TYPE_PNG);
	
	wxToolBar* toolBar = this->CreateToolBar();
	toolBar->AddTool(ID_SkipBackward, "Skip Backward", skipBackwardBitmap, "Skip backards.");
	toolBar->AddTool(ID_SkipForward, "Skip Forward", skipForwardBitmap, "Skip forward.");
	toolBar->AddSeparator();
	toolBar->AddTool(ID_Stop, "Stop", stopBitmap, "Stop.");
	toolBar->AddTool(ID_Play, "Play", playBitmap, "Play!");
	toolBar->AddTool(ID_Pause, "Pause", pauseBitmap, "Pause.");
	toolBar->AddTool(ID_Record, "Record", recordBitmap, "Record.");
	toolBar->Realize();

	// TODO: Add the track sizer to a scrollable control at some point?
	this->trackSizer = new wxBoxSizer(wxVERTICAL);
	this->SetSizer(this->trackSizer);
}

/*virtual*/ AudioToolFrame::~AudioToolFrame()
{
}

void AudioToolFrame::OnClearAll(wxCommandEvent& event)
{
	wxGetApp().ClearAllTrackData();

	std::vector<wxWindow*> windowArray;;

	for (wxSizerItem* item : this->trackSizer->GetChildren())
		windowArray.push_back(item->GetWindow());

	while (true)
	{
		int size = this->trackSizer->GetChildren().size();
		if (size == 0)
			break;

		trackSizer->Remove(size - 1);
	}

	for (wxWindow* window : windowArray)
		window->Destroy();

	this->Refresh();
}

void AudioToolFrame::OnImportAudio(wxCommandEvent& event)
{
	wxFileDialog openFileDialog(this, "Open audio file.", wxEmptyString, wxEmptyString, "(*.wav)|*.wav", wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);
	if (openFileDialog.ShowModal() != wxID_OK)
		return;
			
	wxArrayString filePathArray;
	openFileDialog.GetPaths(filePathArray);
	if (filePathArray.size() == 0)
		return;

	wxArrayString errorArray;

	for(const wxString& filePath : filePathArray)
	{
		AudioFileFormat* audioFileFormat = AudioFileFormat::CreateForFile((const char*)filePath.c_str());
		if (!audioFileFormat)
		{
			errorArray.push_back(wxString::Format("Support loading files of type (%s) does not yet exist.", filePath.c_str()));
			continue;
		}
		
		FileInputStream inputStream(filePath.c_str());

		AudioData* audioData = nullptr;
		std::string error;
		if (!audioFileFormat->ReadFromStream(inputStream, audioData, error))
			errorArray.push_back(wxString::Format("Failed to load file %s: %s", filePath.c_str(), error.c_str()));
		else
		{
			int trackNumber = wxGetApp().GetNumTracks();

			TrackData* trackData = new TrackData();
			trackData->SetName(wxString::Format("Track%d", trackNumber));
			trackData->SetAudioData(audioData);
			wxGetApp().AddTrackData(trackData);

			AudioTrackControl* trackControl = new AudioTrackControl(this, wxID_ANY, trackData->GetName());
			trackControl->SetTrackName(trackData->GetName());

			this->trackSizer->Add(trackControl, 1, wxALL | wxGROW, 0);
		}

		AudioFileFormat::Destroy(audioFileFormat);
	}

	if (errorArray.size() > 0)
		wxGetApp().ShowErrorDialog(errorArray);

	this->Layout();
	this->Refresh();
}

void AudioToolFrame::OnExportAudio(wxCommandEvent& event)
{
	//...
}

void AudioToolFrame::OnSkip(wxCommandEvent& event)
{
}

void AudioToolFrame::OnStop(wxCommandEvent& event)
{
}

void AudioToolFrame::OnPlay(wxCommandEvent& event)
{
}

void AudioToolFrame::OnPause(wxCommandEvent& event)
{
}

void AudioToolFrame::OnRecord(wxCommandEvent& event)
{
}

void AudioToolFrame::OnAbout(wxCommandEvent& event)
{
	wxAboutDialogInfo aboutDialogInfo;

	aboutDialogInfo.SetName("Audio Tool");
	aboutDialogInfo.SetVersion("1.0");
	aboutDialogInfo.SetDescription("This program exercises the AudoDataLib software for testing purposes, but might be useful otherwise.");
	aboutDialogInfo.SetCopyright("Copyright (C) 2024 -- Spencer T. Parkin <spencer.parkin@proton.me>");

	wxAboutBox(aboutDialogInfo);
}

void AudioToolFrame::OnExit(wxCommandEvent& event)
{
	this->Close(true);
}