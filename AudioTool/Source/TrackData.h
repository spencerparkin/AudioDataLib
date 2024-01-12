#pragma once

#include <wx/string.h>
#include <wx/dcclient.h>
#include "FileData.h"
#include "Error.h"

class TrackData
{
public:
	TrackData();
	virtual ~TrackData();

	const wxString& GetName() const { return this->name; }
	void SetName(const wxString& name) { this->name = name; }

	void SetTimeSeconds(double timeSeconds) { this->timeSeconds = timeSeconds; }
	double GetTimeSeconds() const { return this->timeSeconds; }

	void SetSelected(bool selected) { this->selected = selected; }
	bool GetSelected() const { return this->selected; }

	static TrackData* MakeTrackDataFor(AudioDataLib::FileData* fileData);

	virtual void Render(wxPaintDC& paintDC) const = 0;
	virtual bool Process(AudioDataLib::Error& error) = 0;
	virtual bool BeginPlayback(AudioDataLib::Error& error) = 0;
	virtual bool StopPlayback(AudioDataLib::Error& error) = 0;
	virtual bool BeginRecording(AudioDataLib::Error& error) = 0;
	virtual bool StopRecording(AudioDataLib::Error& error) = 0;
	virtual bool GetStatusMessage(std::string& statusMsg) const = 0;

	enum State
	{
		HAPPY,
		PLAYING,
		RECORDING
	};

	virtual State GetState() const = 0;

protected:
	bool selected;
	double timeSeconds;
	wxString name;
};