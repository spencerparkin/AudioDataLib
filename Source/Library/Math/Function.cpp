#include "Function.h"

using namespace AudioDataLib;

//----------------------------- Function -----------------------------

Function::Function()
{
}

/*virtual*/ Function::~Function()
{
}

/*virtual*/ double Function::EvaluateDerivativeAt(double x) const
{
	double dx = 1e-6;
	return (this->EvaluateAt(x + dx) - this->EvaluateAt(x - dx)) / (2.0 * dx);
}

//----------------------------- ConstantFunction -----------------------------

ConstantFunction::ConstantFunction()
{
	this->y = 0.0;
}

/*virtual*/ ConstantFunction::~ConstantFunction()
{
}

/*virtual*/ double ConstantFunction::EvaluateAt(double x) const
{
	return this->y;
}

/*virtual*/ double ConstantFunction::EvaluateDerivativeAt(double x) const
{
	return 0.0;
}

//----------------------------- LinearFallOffFunction -----------------------------

LinearFallOffFunction::LinearFallOffFunction(double decayTimeSeconds)
{
	this->decayTimeSeconds = decayTimeSeconds;
}

/*virtual*/ LinearFallOffFunction::~LinearFallOffFunction()
{
}

/*virtual*/ double LinearFallOffFunction::EvaluateAt(double timeSeconds) const
{
	if (timeSeconds <= 0.0)
		return 1.0;

	if (timeSeconds >= this->decayTimeSeconds)
		return 0.0;

	return 1.0 - timeSeconds / this->decayTimeSeconds;
}