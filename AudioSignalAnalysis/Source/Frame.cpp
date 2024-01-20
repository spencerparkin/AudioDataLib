#include "Frame.h"
#include "Canvas.h"
#include "WaveFileFormat.h"
#include "Error.h"
#include "App.h"
#include "Audio.h"
#include <wx/aboutdlg.h>
#include <wx/sizer.h>
#include <wx/menu.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>

using namespace AudioDataLib;

Frame::Frame(const wxPoint& pos, const wxSize& size) : wxFrame(nullptr, wxID_ANY, "Audio Signal Analysis", pos, size)
{
	wxMenu* fileMenu = new wxMenu();
	fileMenu->Append(new wxMenuItem(fileMenu, ID_ImportAudio, "Import Audio", "Import audio from a selected file."));
	fileMenu->Append(new wxMenuItem(fileMenu, ID_ExportAudio, "Export Audio", "Export audio to disk."));
	fileMenu->AppendSeparator();
	fileMenu->Append(new wxMenuItem(fileMenu, ID_Exit, "Exit", "Go skiing."));

	wxMenu* helpMenu = new wxMenu();
	helpMenu->Append(new wxMenuItem(helpMenu, ID_About, "About", "Show the about-box."));

	wxMenuBar* menuBar = new wxMenuBar();
	menuBar->Append(fileMenu, "File");
	menuBar->Append(helpMenu, "Help");
	this->SetMenuBar(menuBar);

	this->Bind(wxEVT_MENU, &Frame::OnExit, this, ID_Exit);
	this->Bind(wxEVT_MENU, &Frame::OnAbout, this, ID_About);
	this->Bind(wxEVT_MENU, &Frame::OnImportAudio, this, ID_ImportAudio);
	this->Bind(wxEVT_MENU, &Frame::OnExportAudio, this, ID_ExportAudio);

	this->SetStatusBar(new wxStatusBar(this));

	this->canvas = new Canvas(this);

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(this->canvas, 1, wxALL | wxGROW, 0);
	this->SetSizer(sizer);
}

/*virtual*/ Frame::~Frame()
{
}

void Frame::OnImportAudio(wxCommandEvent& event)
{
	wxFileDialog fileDialog(this, "Locate your audio data files.", wxEmptyString, wxEmptyString, "Wave files (*.wav)|*.wav", wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);
	if (wxID_OK != fileDialog.ShowModal())
		return;

	wxArrayString audioFileArray;
	fileDialog.GetPaths(audioFileArray);

	WaveFileFormat fileFormat;
	for (const wxString& audioFile : audioFileArray)
	{
		FileInputStream inputStream(audioFile.c_str());
		if (!inputStream.IsOpen())
			continue;

		Error error;
		FileData* fileData = nullptr;
		if (!fileFormat.ReadFromStream(inputStream, fileData, error))
		{
			wxMessageBox(error.GetMessage(), "Error!", wxICON_ERROR | wxOK, this);
			continue;
		}

		auto audioData = dynamic_cast<AudioData*>(fileData);
		if (!audioData)
		{
			delete audioData;
			continue;
		}

		WaveFormAudio* audio = new WaveFormAudio();
		audio->SetAudioData(audioData);
		wxGetApp().audioArray.push_back(audio);

		this->canvas->FitContent();
	}

	this->Refresh();
}

void Frame::OnExportAudio(wxCommandEvent& event)
{
}

void Frame::OnExit(wxCommandEvent& event)
{
	this->Close(true);
}

void Frame::OnAbout(wxCommandEvent& event)
{
	wxAboutDialogInfo aboutDialogInfo;

	aboutDialogInfo.SetName("Audio Signal Analysis");
	aboutDialogInfo.SetDescription("Load and analyze WAV files using FFTs.");

	wxAboutBox(aboutDialogInfo);
}