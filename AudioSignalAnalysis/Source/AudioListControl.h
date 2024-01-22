#pragma once

#include <wx/listctrl.h>

#define AUDIO_LIST_COLUMN_NAME			0
#define AUDIO_LIST_COLUMN_SIZE			1

class Audio;

class AudioListControl : public wxListCtrl
{
public:
	AudioListControl(wxWindow* parent);
	virtual ~AudioListControl();

	virtual wxString OnGetItemText(long item, long column) const override;

	void OnItemSelected(wxListEvent& event);
	void OnItemActivated(wxListEvent& event);

	void Update();

private:
	const Audio* GetAudio(long item) const;
	const Audio* GetAudio(wxListEvent& event) const;
	Audio* GetAudio(wxListEvent& event);
};