#pragma once

#include <string>
#include <list>
#include <set>
#ifdef _WIN32
#	define WIN32_MEAN_AND_LEAN
#	include <Windows.h>
#endif

class Keyboard
{
public:
	Keyboard();
	virtual ~Keyboard();

	static Keyboard* Create();

	virtual bool Setup() = 0;
	virtual bool Shutdown() = 0;
	virtual bool Process() = 0;

	// These are platform-independent key-codes.
	// These are meant to miss the standard ASCII characters.
	enum class Key : int32_t
	{
		KEY_ESCAPE = 10000,
		KEY_ENTER,
		KEY_TAB,
		KEY_F1,
		KEY_F2,
		KEY_F3,
		KEY_F4,
		KEY_F5,
		KEY_F6,
		KEY_F7,
		KEY_F8,
		KEY_F9,
		KEY_F10,
		KEY_F11,
		KEY_F12,
		KEY_HOME,
		KEY_DEL,
		KEY_INS,
		KEY_PAGE_UP,
		KEY_PAGE_DOWN,
		KEY_END,
		KEY_SHIFT,
		KEY_CTRL,
		KEY_ALT,
		KEY_LEFT,
		KEY_RIGHT,
		KEY_UP,
		KEY_DOWN,
		KEY_CAPS_LOCK,
		KEY_BACKSPACE,
		KEY_SPACEBAR
	};

	class Event
	{
	public:
		Event();
		virtual ~Event();

		enum Type
		{
			UNKNOWN,
			KEY_PRESSED,
			KEY_RELEASED
		};

		Type type;
		int32_t keyCode;
	};

	bool GetKeyboardEvent(Event& event);

protected:

	std::list<Event> keyEventQueue;
};

#ifdef _WIN32

class WindowsKeyboard : public Keyboard
{
public:
	WindowsKeyboard();
	virtual ~WindowsKeyboard();

	virtual bool Setup() override;
	virtual bool Shutdown() override;
	virtual bool Process() override;

private:
	bool TranslateEvent(const KEY_EVENT_RECORD& keyEventRecord, Event& event);

	HANDLE inputHandle;
};

#endif //_WIN32