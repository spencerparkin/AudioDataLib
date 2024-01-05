#pragma once

#include <wx/frame.h>

class AudioToolFrame : public wxFrame
{
public:
	AudioToolFrame(const wxPoint& pos, const wxSize& size);
	virtual ~AudioToolFrame();

	enum
	{
		ID_ImportAudio = wxID_HIGHEST,
		ID_ExportAudio,
		ID_About,
		ID_Exit
	};

	void OnImportAudio(wxCommandEvent& event);
	void OnExportAudio(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnExit(wxCommandEvent& event);
};