#include "Timer.h"

using namespace AudioDataLib;

//--------------------------------- Timer ---------------------------------

Timer::Timer()
{
	this->running = false;
	this->startTimeSeconds = 0.0;
	this->elapsedTimeSeconds = 0.0;
	this->lastTimeSeconds = 0.0;
	this->maxDeltaTimeSeconds = 0.0;
}

/*virtual*/ Timer::~Timer()
{
}

void Timer::Start()
{
	this->running = true;
	this->InitBaseTime();
	this->startTimeSeconds = this->GetCurrentTimeSeconds();
}

void Timer::Stop()
{
	this->GetElapsedTimeSeconds();
	this->running = false;
}

void Timer::Reset()
{
	this->startTimeSeconds = this->GetCurrentTimeSeconds();
	this->elapsedTimeSeconds = 0.0;
}

double Timer::GetElapsedTimeSeconds()
{
	if (this->running)
		this->elapsedTimeSeconds = this->GetCurrentTimeSeconds() - this->startTimeSeconds;

	return this->elapsedTimeSeconds;
}

double Timer::GetDeltaTimeSeconds()
{
	double deltaTimeSeconds = 0.0;
	double currentTimeSeconds = this->GetCurrentTimeSeconds();
	if (this->lastTimeSeconds != 0.0)
		deltaTimeSeconds = currentTimeSeconds - this->lastTimeSeconds;
	this->lastTimeSeconds = currentTimeSeconds;
	if (this->maxDeltaTimeSeconds != 0.0 && deltaTimeSeconds > this->maxDeltaTimeSeconds)
		deltaTimeSeconds = 0.0;		// This is helpful when debugging time-based simulations.
	return deltaTimeSeconds;
}

//--------------------------------- ClockTicksTimer ---------------------------------

ClockTicksTimer::ClockTicksTimer()
{
	this->baseTimeTicks = 0;
}

/*virtual*/ ClockTicksTimer::~ClockTicksTimer()
{
}

/*virtual*/ void ClockTicksTimer::InitBaseTime()
{
	this->baseTimeTicks = ::clock();
}

/*virtual*/ double ClockTicksTimer::GetCurrentTimeSeconds()
{
	clock_t presentTimeTicks = ::clock();
	clock_t currentTimeTicks = presentTimeTicks - this->baseTimeTicks;
	double currentTimeSeconds = double(currentTimeTicks) / double(CLOCKS_PER_SEC);
	return currentTimeSeconds;
}

//--------------------------------- HighResTimer ---------------------------------

HighResTimer::HighResTimer()
{
	this->baseTime = new std::chrono::steady_clock::time_point();
}

/*virtual*/ HighResTimer::~HighResTimer()
{
	delete this->baseTime;
}

/*virtual*/ void HighResTimer::InitBaseTime()
{
	*this->baseTime = std::chrono::high_resolution_clock::now();
}

/*virtual*/ double HighResTimer::GetCurrentTimeSeconds()
{
	std::chrono::steady_clock::time_point presentTime = std::chrono::high_resolution_clock::now();
	unsigned long long currentTimeNanoSeconds = std::chrono::duration_cast<std::chrono::nanoseconds>(presentTime - *this->baseTime).count();
	double currentTimeSeconds = double(currentTimeNanoSeconds) / double(1e+9);
	return currentTimeSeconds;
}

//--------------------------------- SystemClockTimer ---------------------------------

SystemClockTimer::SystemClockTimer()
{
	this->baseTime = new std::chrono::system_clock::time_point();
}

/*virtual*/ SystemClockTimer::~SystemClockTimer()
{
	delete this->baseTime;
}

/*virtual*/ void SystemClockTimer::InitBaseTime()
{
	*this->baseTime = std::chrono::system_clock::now();
}

/*virtual*/ double SystemClockTimer::GetCurrentTimeSeconds()
{
	std::chrono::system_clock::time_point presentTime = std::chrono::system_clock::now();
	unsigned long long currentTimeNanoSeconds = std::chrono::duration_cast<std::chrono::nanoseconds>(presentTime - *this->baseTime).count();
	double currentTimeSeconds = double(currentTimeNanoSeconds) / double(1e+9);
	return currentTimeSeconds;
}