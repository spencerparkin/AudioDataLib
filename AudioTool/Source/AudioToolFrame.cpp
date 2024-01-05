#include "AudioToolFrame.h"
#include <wx/aboutdlg.h>
#include <wx/menu.h>
#include <wx/toolbar.h>

AudioToolFrame::AudioToolFrame(const wxPoint& pos, const wxSize& size) : wxFrame(nullptr, wxID_ANY, "Audio Tool", pos, size)
{
	wxMenu* fileMenu = new wxMenu();
	fileMenu->Append(new wxMenuItem(fileMenu, ID_ImportAudio, "Import Audio", "Import audio from a file on disk."));
	fileMenu->Append(new wxMenuItem(fileMenu, ID_ExportAudio, "Export Audio", "Export audio to a file on disk."));
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
	this->Bind(wxEVT_MENU, &AudioToolFrame::OnSkip, this, ID_SkipBackward);
	this->Bind(wxEVT_MENU, &AudioToolFrame::OnSkip, this, ID_SkipForward);
	this->Bind(wxEVT_MENU, &AudioToolFrame::OnStop, this, ID_Stop);
	this->Bind(wxEVT_MENU, &AudioToolFrame::OnPlay, this, ID_Play);
	this->Bind(wxEVT_MENU, &AudioToolFrame::OnPause, this, ID_Pause);
	this->Bind(wxEVT_MENU, &AudioToolFrame::OnAbout, this, ID_About);
	this->Bind(wxEVT_MENU, &AudioToolFrame::OnExit, this, ID_Exit);

	wxBitmap skipBackwardBitmap, skipForwardBitmap;
	wxBitmap stopBitmap, playBitmap, pauseBitmap;

	skipBackwardBitmap.LoadFile(wxGetCwd() + "/Images/skip_backward.png", wxBITMAP_TYPE_PNG);
	skipForwardBitmap.LoadFile(wxGetCwd() + "/Images/skip_forward.png", wxBITMAP_TYPE_PNG);
	stopBitmap.LoadFile(wxGetCwd() + "/Images/stop.png", wxBITMAP_TYPE_PNG);
	playBitmap.LoadFile(wxGetCwd() + "/Images/play.png", wxBITMAP_TYPE_PNG);
	pauseBitmap.LoadFile(wxGetCwd() + "/Images/pause.png", wxBITMAP_TYPE_PNG);
	
	wxToolBar* toolBar = this->CreateToolBar();
	toolBar->AddTool(ID_SkipBackward, "Skip Backward", skipBackwardBitmap, "Skip backards.");
	toolBar->AddTool(ID_SkipForward, "Skip Forward", skipForwardBitmap, "Skip forward.");
	toolBar->AddSeparator();
	toolBar->AddTool(ID_Stop, "Stop", stopBitmap, "Stop.");
	toolBar->AddTool(ID_Play, "Play", playBitmap, "Play!");
	toolBar->AddTool(ID_Pause, "Pause", pauseBitmap, "Pause.");
	toolBar->Realize();
}

/*virtual*/ AudioToolFrame::~AudioToolFrame()
{
}

void AudioToolFrame::OnImportAudio(wxCommandEvent& event)
{
	//...
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