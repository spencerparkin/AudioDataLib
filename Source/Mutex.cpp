#include "Mutex.h"

using namespace AudioDataLib;

//-------------------------- Mutex --------------------------

Mutex::Mutex()
{
}

/*virtual*/ Mutex::~Mutex()
{
}

//-------------------------- StandardMutex --------------------------

StandardMutex::StandardMutex()
{
	this->mutex = new std::mutex();
}

/*virtual*/ StandardMutex::~StandardMutex()
{
	delete this->mutex;
}

/*virtual*/ void StandardMutex::Lock()
{
	this->mutex->lock();
}

/*virtual*/ void StandardMutex::Unlock()
{
	this->mutex->unlock();
}

//-------------------------- MutexScopeLock --------------------------

MutexScopeLock::MutexScopeLock(Mutex* mutex)
{
	this->mutex = mutex;
	this->mutex->Lock();
}

/*virtual*/ MutexScopeLock::~MutexScopeLock()
{
	this->mutex->Unlock();
}