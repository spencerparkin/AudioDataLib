#pragma once

#include "Common.h"

namespace AudioDataLib
{
	class AUDIO_DATA_LIB_API Mutex
	{
	public:
		Mutex();
		virtual ~Mutex();

		virtual void Lock();
		virtual void Unlock();
	};

	class AUDIO_DATA_LIB_API MutexScopeLock
	{
	public:
		MutexScopeLock(Mutex* mutex);
		virtual ~MutexScopeLock();

	private:
		Mutex* mutex;
	};
}