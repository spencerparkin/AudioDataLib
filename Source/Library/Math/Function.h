#pragma once

#include "Common.h"

namespace AudioDataLib
{
	/**
	 * @brief These are real-valued functions of a real variable.
	 * 
	 * Functions are as fundamental to mathematics as numbers, so it's not surprising
	 * to find the concept useful enough to abstract them in this manner.
	 */
	class AUDIO_DATA_LIB_API Function
	{
	public:
		Function();
		virtual ~Function();

		/**
		 * Derived classes should override this method to provide a means of evaluating this function
		 * at the given value, typically representative of time in this library.
		 */
		virtual double EvaluateAt(double x) const = 0;

		/**
		 * Derived classes can override this method to provide a means of evaluating the derivative of
		 * this function at the given value.  By default, a central differencing approximation method
		 * is implimented here.  If a more accurate method is possible, it should be implimented by
		 * the derived class.
		 */
		virtual double EvaluateDerivativeAt(double x) const;
	};

	/**
	 * The simplest kind of function, this one return the same output value for all input values.
	 */
	class AUDIO_DATA_LIB_API ConstantFunction : public Function
	{
	public:
		ConstantFunction();
		virtual ~ConstantFunction();

		virtual double EvaluateAt(double x) const override;
		virtual double EvaluateDerivativeAt(double x) const override;

		double y;
	};

	/**
	 * This function is used to scale a wave-form over time so that it doesn't just obrubtly end,
	 * causing an unwanted sound artifact.
	 */
	class AUDIO_DATA_LIB_API LinearFallOffFunction : public Function
	{
	public:
		LinearFallOffFunction(double decayTimeSeconds);
		virtual ~LinearFallOffFunction();

		virtual double EvaluateAt(double timeSeconds) const override;
		
		double decayTimeSeconds;		///< This is the time, in seconds, it will take for the function to go to zero.
	};
}