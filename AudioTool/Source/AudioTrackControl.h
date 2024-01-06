#pragma once

#include <wx/control.h>
#include <memory>
#include "TrackData.h"

#define ATC_IDEAL_CONTROL_HEIGHT		100

enum AudioTrackControlStyles
{
	ATC_DEFAULT_STYLE = 1
};

// This custom control provides an interface to a single chunk of audio data.
class AudioTrackControl : public wxControl
{
public:
	AudioTrackControl();
	AudioTrackControl(
		wxWindow* parent,
		wxWindowID winId,
		const wxString& label,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = ATC_DEFAULT_STYLE,
		const wxValidator& validator = wxDefaultValidator,
		const wxString& name = "AudioTrackControlWidget");

	bool Create(
		wxWindow* parent,
		wxWindowID winId,
		const wxString& label,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = ATC_DEFAULT_STYLE,
		const wxValidator& validator = wxDefaultValidator,
		const wxString& name = "AudioTrackControlWidget");

	void SetTrackName(const wxString& trackName) { this->trackName = trackName; }
	const wxString& GetTrackName() const { return this->trackName; }

	TrackData* GetTrackData();
	const TrackData* GetTrackData() const;

protected:

	void Init();

	virtual wxSize DoGetBestSize() const override;
	virtual wxSize DoGetBestClientSize() const override;
	
	void OnPaint(wxPaintEvent& event);
	void OnSize(wxSizeEvent& event);

private:
	wxString trackName;

	wxDECLARE_DYNAMIC_CLASS(AudioTrackControl);
	wxDECLARE_EVENT_TABLE();
};