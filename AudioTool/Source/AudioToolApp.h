#pragma once

#include <wx/app.h>
#include <vector>
#include <memory>
#include "TrackData.h"

class AudioToolFrame;

class AudioToolApp : public wxApp
{
public:
	AudioToolApp();
	virtual ~AudioToolApp();

	virtual bool OnInit(void) override;
	virtual int OnExit(void) override;

	AudioToolFrame* GetFrame() { return this->frame; }
	int GetNumTracks() { return this->trackDataArray.size(); }
	TrackData* GetTrackData(int i);
	void AddTrackData(TrackData* trackData);
	void RemoveTrackData(int i);
	TrackData* FindTrackData(const wxString& name);
	void ClearAllTrackData();
	void GetAllTracks(std::vector<TrackData*>& selectedTracksArray, bool thatAreSelected);
	void ClearAllTrackSelection();

	void ShowErrorDialog(const wxArrayString& errorArray);

private:

	std::vector<TrackData*> trackDataArray;

	AudioToolFrame* frame;
};

wxDECLARE_APP(AudioToolApp);