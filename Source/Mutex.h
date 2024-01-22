#pragma once

#include "Common.h"
#include <mutex>

namespace AudioDataLib
{
	class AUDIO_DATA_LIB_API Mutex
	{
	public:
		Mutex();
		virtual ~Mutex();

		virtual void Lock() = 0;
		virtual void Unlock() = 0;
	};

	class AUDIO_DATA_LIB_API StandardMutex : public Mutex
	{
	public:
		StandardMutex();
		virtual ~StandardMutex();

		virtual void Lock() override;
		virtual void Unlock() override;

	private:
		std::mutex* mutex;
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