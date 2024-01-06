#include "AudioToolApp.h"
#include "AudioToolFrame.h"
#include <wx/image.h>
#include <wx/msgdlg.h>

wxIMPLEMENT_APP(AudioToolApp);

AudioToolApp::AudioToolApp()
{
	this->frame = nullptr;
}

/*virtual*/ AudioToolApp::~AudioToolApp()
{
}

/*virtual*/ bool AudioToolApp::OnInit(void)
{
	if (!wxApp::OnInit())
		return false;

	wxInitAllImageHandlers();

	this->frame = new AudioToolFrame(wxDefaultPosition, wxSize(800, 600));
	this->frame->Show();

	return true;
}

/*virtual*/ int AudioToolApp::OnExit(void)
{
	this->ClearAllTrackData();

	return 0;
}

void AudioToolApp::ClearAllTrackData()
{
	for (TrackData* trackData : this->trackDataArray)
		delete trackData;

	this->trackDataArray.clear();
}

TrackData* AudioToolApp::GetTrackData(int i)
{
	if (i < 0 || i >= (signed)this->trackDataArray.size())
		return nullptr;

	return this->trackDataArray[i];
}

void AudioToolApp::AddTrackData(TrackData* trackData)
{
	this->trackDataArray.push_back(trackData);
}

void AudioToolApp::RemoveTrackData(int i)
{
	//...
}

TrackData* AudioToolApp::FindTrackData(const wxString& name)
{
	for (TrackData* trackData : this->trackDataArray)
		if (trackData->GetName() == name)
			return trackData;

	return nullptr;
}

void AudioToolApp::ShowErrorDialog(const wxArrayString& errorArray)
{
	wxString errorMsg = wxString::Format("%d error encountered...\n\n", errorArray.size());
	for (int i = 0; i < (signed)errorArray.size(); i++)
		errorMsg += errorArray[i] + "\n";

	wxMessageBox(errorMsg, "Error!", wxICON_ERROR | wxOK, this->frame);
}