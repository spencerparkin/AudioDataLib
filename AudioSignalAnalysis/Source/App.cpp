#include "App.h"
#include "Frame.h"
#include "Audio.h"

wxIMPLEMENT_APP(App);

App::App()
{
	this->frame = nullptr;
}

/*virtual*/ App::~App()
{
	this->Clear();
}

/*virtual*/ bool App::OnInit(void)
{
	if (!wxApp::OnInit())
		return false;

	this->frame = new Frame(wxDefaultPosition, wxSize(800, 800));
	this->frame->Show();

	return true;
}

/*virtual*/ int App::OnExit(void)
{
	return 0;
}

void App::Clear()
{
	for (Audio* audio : this->audioArray)
		delete audio;

	this->audioArray.clear();
}

bool App::GetSelectedAudio(std::vector<Audio*>& selectedAudioArray)
{
	selectedAudioArray.clear();
	for (Audio* audio : this->audioArray)
		if (audio->IsSelected())
			selectedAudioArray.push_back(audio);

	return selectedAudioArray.size() > 0;
}