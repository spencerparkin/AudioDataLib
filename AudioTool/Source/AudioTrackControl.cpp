#include "AudioTrackControl.h"
#include "AudioToolApp.h"
#include "TrackData.h"
#include <wx/dcclient.h>

wxIMPLEMENT_DYNAMIC_CLASS(AudioTrackControl, wxControl);

wxBEGIN_EVENT_TABLE(AudioTrackControl, wxControl)
wxEND_EVENT_TABLE()

AudioTrackControl::AudioTrackControl()
{
	this->Init();
}

AudioTrackControl::AudioTrackControl(
	wxWindow* parent,
	wxWindowID winId,
	const wxString& label,
	const wxPoint& pos /*= wxDefaultPosition*/,
	const wxSize& size /*= wxDefaultSize*/,
	long style /*= ATC_DEFAULT_STYLE*/,
	const wxValidator& validator /*= wxDefaultValidator*/,
	const wxString& name /*= "AudioTrackControlWidget"*/)
{
	this->Init();

	this->Create(parent, winId, label, pos, size, style, validator, name);
}

bool AudioTrackControl::Create(
	wxWindow* parent,
	wxWindowID winId,
	const wxString& label,
	const wxPoint& pos /*= wxDefaultPosition*/,
	const wxSize& size /*= wxDefaultSize*/,
	long style /*= ATC_DEFAULT_STYLE*/,
	const wxValidator& validator /*= wxDefaultValidator*/,
	const wxString& name /*= "AudioTrackControlWidget"*/)
{
	//...

	return wxControl::Create(parent, winId, pos, size, style, validator, name);
}

void AudioTrackControl::Init()
{
	this->Bind(wxEVT_PAINT, &AudioTrackControl::OnPaint, this);
	this->Bind(wxEVT_SIZE, &AudioTrackControl::OnSize, this);

	this->SetMaxClientSize(wxSize(-1, ATC_IDEAL_CONTROL_HEIGHT));
}

/*virtual*/ wxSize AudioTrackControl::DoGetBestSize() const
{
	return wxControl::DoGetBestSize();
}

/*virtual*/ wxSize AudioTrackControl::DoGetBestClientSize() const
{
	wxSize size = wxControl::DoGetBestClientSize();

	int width = this->GetParent()->GetSize().GetWidth();
	size.SetWidth(width);

	size.SetHeight(ATC_IDEAL_CONTROL_HEIGHT);

	return size;
}

void AudioTrackControl::OnPaint(wxPaintEvent& event)
{
	wxPaintDC paintDC(this);

	const TrackData* trackData = wxGetApp().FindTrackData(this->trackName);
	if (trackData)
	{
		//...
	}

	wxSize clientAreaSize = paintDC.GetSize();

	wxRect rect(clientAreaSize);

	paintDC.SetPen(*wxGREEN_PEN);
	paintDC.DrawRectangle(rect);

	wxPoint pointA(rect.x, rect.y);
	wxPoint pointB(rect.x + rect.width, rect.y + rect.height);
	paintDC.DrawLine(pointA, pointB);
}

void AudioTrackControl::OnSize(wxSizeEvent& event)
{
	this->Refresh();
}