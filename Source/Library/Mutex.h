#pragma once

#include "Common.h"
#include <mutex>

namespace AudioDataLib
{
	/**
	 * @brief This class provides a mutex interface for thread synchronization.
	 * 
	 * No threads are created or destroyed by AudioDataLib (as of this writing), but it can still be thread-safe
	 * or thread-aware in many cases where it's typical for the user to call different parts
	 * of the API from different threads.
	 */
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