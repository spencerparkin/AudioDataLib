#pragma once

#include "Common.h"

namespace AudioDataLib
{
	class AUDIO_DATA_LIB_API Function
	{
	public:
		Function();
		virtual ~Function();

		virtual double EvaluateAt(double x) const = 0;
		virtual double EvaluateDerivativeAt(double x) const;
	};

	class AUDIO_DATA_LIB_API ConstantFunction : public Function
	{
	public:
		ConstantFunction();
		virtual ~ConstantFunction();

		virtual double EvaluateAt(double x) const override;
		virtual double EvaluateDerivativeAt(double x) const override;

		double y;
	};

	class AUDIO_DATA_LIB_API LinearFallOffFunction : public Function
	{
	public:
		LinearFallOffFunction(double decayTimeSeconds);
		virtual ~LinearFallOffFunction();

		virtual double EvaluateAt(double timeSeconds) const override;
		
		double decayTimeSeconds;
	};
}