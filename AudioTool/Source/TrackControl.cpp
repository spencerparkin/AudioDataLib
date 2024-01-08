#include "TrackControl.h"
#include "AudioToolApp.h"
#include "TrackData.h"
#include <wx/dcclient.h>

wxIMPLEMENT_DYNAMIC_CLASS(TrackControl, wxControl);

wxBEGIN_EVENT_TABLE(TrackControl, wxControl)
wxEND_EVENT_TABLE()

TrackControl::TrackControl()
{
	this->Init();
}

TrackControl::TrackControl(
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

bool TrackControl::Create(
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

void TrackControl::Init()
{
	this->Bind(wxEVT_PAINT, &TrackControl::OnPaint, this);
	this->Bind(wxEVT_SIZE, &TrackControl::OnSize, this);

	this->SetMaxClientSize(wxSize(-1, TC_IDEAL_CONTROL_HEIGHT));
}

/*virtual*/ wxSize TrackControl::DoGetBestSize() const
{
	return wxControl::DoGetBestSize();
}

/*virtual*/ wxSize TrackControl::DoGetBestClientSize() const
{
	wxSize size = wxControl::DoGetBestClientSize();

	int width = this->GetParent()->GetSize().GetWidth();
	size.SetWidth(width);

	size.SetHeight(TC_IDEAL_CONTROL_HEIGHT);

	return size;
}

void TrackControl::OnPaint(wxPaintEvent& event)
{
	wxPaintDC paintDC(this);

	const TrackData* trackData = this->GetTrackData();
	if (trackData)
		trackData->Render(paintDC);

	wxSize clientAreaSize = paintDC.GetSize();

	wxRect rect(clientAreaSize);

	paintDC.SetPen(*wxGREEN_PEN);
	paintDC.DrawRectangle(rect);

	wxPoint pointA(rect.x, rect.y);
	wxPoint pointB(rect.x + rect.width, rect.y + rect.height);
	paintDC.DrawLine(pointA, pointB);
}

void TrackControl::OnSize(wxSizeEvent& event)
{
	this->Refresh();
}

TrackData* TrackControl::GetTrackData()
{
	return wxGetApp().FindTrackData(this->trackName);
}

const TrackData* TrackControl::GetTrackData() const
{
	return wxGetApp().FindTrackData(this->trackName);
}