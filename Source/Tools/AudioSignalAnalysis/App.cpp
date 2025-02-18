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

	this->frame = new Frame(wxDefaultPosition, wxSize(1400, 800));
	this->frame->Show();

	return true;
}

/*virtual*/ int App::OnExit(void)
{
	return 0;
}

void App::Clear()
{
	this->audioArray.clear();
}

void App::AddAudio(std::shared_ptr<Audio> audio)
{
	this->audioArray.push_back(audio);
}

bool App::GetFlaggedAudio(std::vector<Audio*>& foundAudioArray, uint32_t flag)
{
	foundAudioArray.clear();
	for (auto& audio : this->audioArray)
		if ((audio->GetFlags() & flag) != 0)
			foundAudioArray.push_back(audio.get());

	return foundAudioArray.size() > 0;
}