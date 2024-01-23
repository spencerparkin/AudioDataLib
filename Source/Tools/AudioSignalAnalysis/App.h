#pragma once

#include <wx/app.h>

class Frame;
class Audio;

class App : public wxApp
{
public:
	App();
	virtual ~App();

	virtual bool OnInit(void) override;
	virtual int OnExit(void) override;

	void Clear();
	void AddAudio(Audio* audio);
	const std::vector<Audio*>& GetAudioArray() const { return this->audioArray; }
	bool GetFlaggedAudio(std::vector<Audio*>& foundAudioArray, uint32_t flag);
	Frame* GetFrame() { return this->frame; }

private:
	Frame* frame;
	std::vector<Audio*> audioArray;
};

wxDECLARE_APP(App);