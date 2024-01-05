#pragma once

#include <wx/app.h>

class AudioToolFrame;

class AudioToolApp : public wxApp
{
public:
	AudioToolApp();
	virtual ~AudioToolApp();

	virtual bool OnInit(void) override;
	virtual int OnExit(void) override;

	AudioToolFrame* GetFrame() { return this->frame; }

private:
	AudioToolFrame* frame;
};

wxDECLARE_APP(AudioToolApp);