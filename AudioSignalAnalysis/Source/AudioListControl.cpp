#include "AudioListControl.h"
#include "App.h"
#include "Audio.h"
#include "Frame.h"

AudioListControl::AudioListControl(wxWindow* parent) : wxListCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_VIRTUAL | wxLC_REPORT)
{
	this->AppendColumn("Name");
	this->AppendColumn("Size");

	this->Bind(wxEVT_LIST_ITEM_SELECTED, &AudioListControl::OnItemSelected, this);
	this->Bind(wxEVT_LIST_ITEM_ACTIVATED, &AudioListControl::OnItemActivated, this);
}

/*virtual*/ AudioListControl::~AudioListControl()
{
}

void AudioListControl::Update()
{
	uint32_t numAudios = wxGetApp().GetAudioArray().size();
	this->SetItemCount(numAudios);
	this->Refresh();
}

/*virtual*/ wxString AudioListControl::OnGetItemText(long item, long column) const
{
	const Audio* audio = this->GetAudio(item);
	if (!audio)
		return "?";

	return audio->GetColumnInfo(column);
}

void AudioListControl::OnItemSelected(wxListEvent& event)
{
	for (Audio* audio : wxGetApp().GetAudioArray())
		audio->SetFlags(audio->GetFlags() & ~AUDIO_FLAG_SELECTED);

	Audio* selectedAudio = this->GetAudio(event);
	if (selectedAudio)
		selectedAudio->SetFlags(selectedAudio->GetFlags() | AUDIO_FLAG_SELECTED);

	wxGetApp().GetFrame()->Refresh();
}

void AudioListControl::OnItemActivated(wxListEvent& event)
{
	Audio* audio = this->GetAudio(event);
	if (!audio)
		return;

	uint32_t flags = audio->GetFlags();
	if((flags & AUDIO_FLAG_VISIBLE) == 0)
		audio->SetFlags(flags | AUDIO_FLAG_VISIBLE);
	else
		audio->SetFlags(flags & ~AUDIO_FLAG_VISIBLE);

	wxGetApp().GetFrame()->Refresh();
}

const Audio* AudioListControl::GetAudio(wxListEvent& event) const
{
	return const_cast<AudioListControl*>(this)->GetAudio(event);
}

const Audio* AudioListControl::GetAudio(long item) const
{
	const std::vector<Audio*>& audioArray = wxGetApp().GetAudioArray();
	if (0 <= item && item <= (signed)audioArray.size())
		return audioArray[item];

	return nullptr;
}

Audio* AudioListControl::GetAudio(wxListEvent& event)
{
	// This is dumb, but whatever for now.
	// It is not clear to me at all how to get the application's data from the item without this search.
	// You can cast the "event.GetItem()" call to a long integer in some cases, but not all.
	// Also, *when* do you assign user-data to a list-item when using the list control in virtual mode?
	for (Audio* audio : wxGetApp().GetAudioArray())
		if (event.GetItem().GetText() == audio->GetColumnInfo(AUDIO_LIST_COLUMN_NAME))
			return audio;

	return nullptr;
}