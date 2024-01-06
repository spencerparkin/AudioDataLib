#pragma once

#include <wx/frame.h>
#include <wx/sizer.h>

class AudioTrackControl;

class AudioToolFrame : public wxFrame
{
public:
	AudioToolFrame(const wxPoint& pos, const wxSize& size);
	virtual ~AudioToolFrame();

	enum
	{
		ID_ImportAudio = wxID_HIGHEST,
		ID_ExportAudio,
		ID_ClearAll,
		ID_SkipBackward,
		ID_SkipForward,
		ID_Stop,
		ID_Play,
		ID_Pause,
		ID_Record,
		ID_About,
		ID_Exit
	};

	void OnImportAudio(wxCommandEvent& event);
	void OnExportAudio(wxCommandEvent& event);
	void OnClearAll(wxCommandEvent& event);
	void OnSkip(wxCommandEvent& event);
	void OnStop(wxCommandEvent& event);
	void OnPlay(wxCommandEvent& event);
	void OnPause(wxCommandEvent& event);
	void OnRecord(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnExit(wxCommandEvent& event);

	wxBoxSizer* trackSizer;
};