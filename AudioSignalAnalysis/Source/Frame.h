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
		ID_ExportAudio,
		ID_Clear,
		ID_GenerateFrequencyGraph,
		ID_MakeSound
	};

	void OnExit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnImportAudio(wxCommandEvent& event);
	void OnExportAudio(wxCommandEvent& event);
	void OnClear(wxCommandEvent& event);
	void OnGenerateFrequencyGraph(wxCommandEvent& event);
	void OnMakeSound(wxCommandEvent& event);

private:

	Canvas* canvas;
};