#pragma once

#include <wx/control.h>
#include <memory>
#include "TrackData.h"

#define TC_IDEAL_CONTROL_HEIGHT		100

enum TrackControlStyles
{
	TC_DEFAULT_STYLE = 1
};

// This custom control provides an interface to a single chunk of audio data.
class TrackControl : public wxControl
{
public:
	TrackControl();
	TrackControl(
		wxWindow* parent,
		wxWindowID winId,
		const wxString& label,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = TC_DEFAULT_STYLE,
		const wxValidator& validator = wxDefaultValidator,
		const wxString& name = "TrackControlWidget");

	bool Create(
		wxWindow* parent,
		wxWindowID winId,
		const wxString& label,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = TC_DEFAULT_STYLE,
		const wxValidator& validator = wxDefaultValidator,
		const wxString& name = "TrackControlWidget");

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
	void OnLeftMouseButtonDown(wxMouseEvent& event);

private:
	wxString trackName;

	wxDECLARE_DYNAMIC_CLASS(TrackControl);
	wxDECLARE_EVENT_TABLE();
};