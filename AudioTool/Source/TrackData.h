#pragma once

#include <wx/string.h>
#include <wx/dcclient.h>
#include "FileData.h"

class TrackData
{
public:
	TrackData();
	virtual ~TrackData();

	const wxString& GetName() const { return this->name; }
	void SetName(const wxString& name) { this->name = name; }

	static TrackData* MakeTrackDataFor(AudioDataLib::FileData* fileData);

	virtual void Render(wxPaintDC& paintDC) const = 0;

protected:
	wxString name;
};