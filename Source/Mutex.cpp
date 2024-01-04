#include "Mutex.h"

using namespace AudioDataLib;

//-------------------------- Mutex --------------------------

Mutex::Mutex()
{
}

/*virtual*/ Mutex::~Mutex()
{
}

/*virtual*/ void Mutex::Lock()
{
	// We do nothing by default.  It is up to the user
	// of the library to provide a mutex implementation.
}

/*virtual*/ void Mutex::Unlock()
{
	// See above comment.
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