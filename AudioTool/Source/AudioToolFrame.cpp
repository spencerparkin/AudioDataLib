#include "AudioToolFrame.h"
#include <wx/aboutdlg.h>
#include <wx/menu.h>

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
	this->Bind(wxEVT_MENU, &AudioToolFrame::OnAbout, this, ID_About);
	this->Bind(wxEVT_MENU, &AudioToolFrame::OnExit, this, ID_Exit);
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