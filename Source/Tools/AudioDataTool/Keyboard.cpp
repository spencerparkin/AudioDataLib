#include "Keyboard.h"
#include "AudioDataLib/ErrorSystem.h"

//---------------------------- Keyboard ----------------------------

Keyboard::Keyboard()
{
}

/*virtual*/ Keyboard::~Keyboard()
{
}

/*static*/ Keyboard* Keyboard::Create()
{
#ifdef _WIN32
	return new WindowsKeyboard();
#endif //_WIN32
	return nullptr;
}

bool Keyboard::GetKeyboardEvent(Event& event)
{
	if (this->keyEventQueue.size() == 0)
		return false;

	std::list<Event>::iterator iter = this->keyEventQueue.begin();
	event = *iter;
	this->keyEventQueue.pop_back();
	return true;
}

//---------------------------- Keyboard::Event ----------------------------

Keyboard::Event::Event()
{
	this->type = Type::UNKNOWN;
	this->keyCode = 0;
}

/*virtual*/ Keyboard::Event::~Event()
{
}

//---------------------------- WindowsKeyboard ----------------------------

#ifdef _WIN32

WindowsKeyboard::WindowsKeyboard()
{
	this->inputHandle = INVALID_HANDLE_VALUE;
}

/*virtual*/ WindowsKeyboard::~WindowsKeyboard()
{
}

/*virtual*/ bool WindowsKeyboard::Setup()
{
	if (this->inputHandle != INVALID_HANDLE_VALUE)
	{
		AudioDataLib::ErrorSystem::Get()->Add("Keyboard handle already setup!");
		return false;
	}

	this->inputHandle = ::GetStdHandle(STD_INPUT_HANDLE);
	if (this->inputHandle == INVALID_HANDLE_VALUE)
	{
		AudioDataLib::ErrorSystem::Get()->Add("Failed to open windows keyboard.");
		return false;
	}

	return true;
}

/*virtual*/ bool WindowsKeyboard::Shutdown()
{
	if (this->inputHandle)
	{
		// I don't think we're supposed to close the handle.
		//::CloseHandle(this->inputHandle);
		this->inputHandle = INVALID_HANDLE_VALUE;
	}

	return true;
}

/*virtual*/ bool WindowsKeyboard::Process()
{
	if (this->inputHandle == INVALID_HANDLE_VALUE)
		return false;

	if (WAIT_OBJECT_0 == ::WaitForSingleObject(this->inputHandle, 0))
	{
		INPUT_RECORD inputRecord;
		DWORD numReadInputRecords = 0;
		::ReadConsoleInput(this->inputHandle, &inputRecord, 1, &numReadInputRecords);
		if (numReadInputRecords == 1)
		{
			if (inputRecord.EventType == KEY_EVENT)
			{
				Event event;
				if (this->TranslateEvent(inputRecord.Event.KeyEvent, event))
					this->keyEventQueue.push_back(event);
			}
		}
	}

	return true;
}

bool WindowsKeyboard::TranslateEvent(const KEY_EVENT_RECORD& keyEventRecord, Event& event)
{
	if (keyEventRecord.bKeyDown)
		event.type = Event::Type::KEY_PRESSED;
	else
		event.type = Event::Type::KEY_RELEASED;

	switch (keyEventRecord.wVirtualKeyCode)
	{
		case VK_SHIFT: event.keyCode = (int32_t)Key::KEY_SHIFT; break;
		case VK_BACK: event.keyCode = (int32_t)Key::KEY_BACKSPACE; break;
		case VK_TAB: event.keyCode = (int32_t)Key::KEY_TAB; break;
		case VK_RETURN: event.keyCode = (int32_t)Key::KEY_ENTER; break;
		case VK_CONTROL: event.keyCode = (int32_t)Key::KEY_CTRL; break;
		case VK_SPACE: event.keyCode = (int32_t)Key::KEY_SPACEBAR; break;
		case VK_DELETE: event.keyCode = (int32_t)Key::KEY_DEL; break;
		case VK_INSERT: event.keyCode = (int32_t)Key::KEY_INS; break;
		case VK_PRIOR: event.keyCode = (int32_t)Key::KEY_PAGE_UP; break;
		case VK_NEXT: event.keyCode = (int32_t)Key::KEY_PAGE_DOWN; break;
		case VK_UP: event.keyCode = (int32_t)Key::KEY_UP; break;
		case VK_DOWN: event.keyCode = (int32_t)Key::KEY_DOWN; break;
		case VK_LEFT: event.keyCode = (int32_t)Key::KEY_LEFT; break;
		case VK_RIGHT: event.keyCode = (int32_t)Key::KEY_RIGHT; break;
		case VK_ESCAPE: event.keyCode = (int32_t)Key::KEY_ESCAPE; break;
		case VK_F1: event.keyCode = (int32_t)Key::KEY_F1; break;
		case VK_F2: event.keyCode = (int32_t)Key::KEY_F2; break;
		case VK_F3: event.keyCode = (int32_t)Key::KEY_F3; break;
		case VK_F4: event.keyCode = (int32_t)Key::KEY_F4; break;
		case VK_F5: event.keyCode = (int32_t)Key::KEY_F5; break;
		case VK_F6: event.keyCode = (int32_t)Key::KEY_F6; break;
		case VK_F7: event.keyCode = (int32_t)Key::KEY_F7; break;
		case VK_F8: event.keyCode = (int32_t)Key::KEY_F8; break;
		case VK_F9: event.keyCode = (int32_t)Key::KEY_F9; break;
		case VK_F10: event.keyCode = (int32_t)Key::KEY_F10; break;
		case VK_F11: event.keyCode = (int32_t)Key::KEY_F11; break;
		case VK_F12: event.keyCode = (int32_t)Key::KEY_F12; break;
		default:
		{
			event.keyCode = keyEventRecord.uChar.AsciiChar;
			break;
		}
	}

	return true;
}

#endif //_WIN32