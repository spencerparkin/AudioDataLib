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

	Frame* frame;

	std::vector<Audio*> audioArray;
};

wxDECLARE_APP(App);