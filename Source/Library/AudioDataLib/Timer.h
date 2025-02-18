#pragma once

#include "AudioDataLib/Common.h"

namespace AudioDataLib
{
	/**
	 * @\brief This class is sometimes used to keep time in a more formal way.
	 */
	class AUDIO_DATA_LIB_API Timer
	{
	public:
		Timer();
		virtual ~Timer();

		void Start();
		void Stop();
		void Reset();
		double GetElapsedTimeSeconds();
		double GetDeltaTimeSeconds();

		virtual void InitBaseTime() = 0;
		virtual double GetCurrentTimeSeconds() = 0;

		bool IsRunning() const { return this->running; }

		void SetMaxDeltaTimeSeconds(double maxDeltaTimeSeconds) { this->maxDeltaTimeSeconds = maxDeltaTimeSeconds; }
		double GetMaxDeltaTimeSeconds() const { return this->maxDeltaTimeSeconds; }

	protected:
		bool running;
		double startTimeSeconds;
		double elapsedTimeSeconds;
		double lastTimeSeconds;
		double maxDeltaTimeSeconds;
	};

	class AUDIO_DATA_LIB_API ClockTicksTimer : public Timer
	{
	public:
		ClockTicksTimer();
		virtual ~ClockTicksTimer();

		virtual void InitBaseTime() override;
		virtual double GetCurrentTimeSeconds() override;

	private:
		clock_t baseTimeTicks;
	};

	class AUDIO_DATA_LIB_API HighResTimer : public Timer
	{
	public:
		HighResTimer();
		virtual ~HighResTimer();

		virtual void InitBaseTime() override;
		virtual double GetCurrentTimeSeconds() override;

	private:
		std::chrono::steady_clock::time_point* baseTime;
	};

	class AUDIO_DATA_LIB_API SystemClockTimer : public Timer
	{
	public:
		SystemClockTimer();
		virtual ~SystemClockTimer();

		virtual void InitBaseTime() override;
		virtual double GetCurrentTimeSeconds() override;

	private:
		std::chrono::system_clock::time_point* baseTime;
	};
}