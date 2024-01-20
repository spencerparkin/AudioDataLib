#pragma once

#include <wx/frame.h>

class Canvas;

class Frame : public wxFrame
{
public:
	Frame(const wxPoint& pos, const wxSize& size);
	virtual ~Frame();

	enum
	{
		ID_Exit = wxID_HIGHEST,
		ID_About,
		ID_ImportAudio,
		ID_ExportAudio
	};

	void OnExit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnImportAudio(wxCommandEvent& event);
	void OnExportAudio(wxCommandEvent& event);

private:

	Canvas* canvas;
};