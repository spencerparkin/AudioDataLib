#include "AudioToolApp.h"
#include "AudioToolFrame.h"
#include <wx/image.h>

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
	return 0;
}