#include "ComplexVector.h"
#include "Error.h"

using namespace AudioDataLib;

ComplexVector::ComplexVector()
{
	this->complexArray = new std::vector<ComplexNumber>();
}

ComplexVector::ComplexVector(const ComplexVector& complexVector)
{
	this->complexArray = new std::vector<ComplexNumber>();
	*this = complexVector;
}

/*virtual*/ ComplexVector::~ComplexVector()
{
	delete this->complexArray;
}

void ComplexVector::operator=(const ComplexVector& complexVector)
{
	this->complexArray->clear();
	for (const ComplexNumber& complexNumber : *complexVector.complexArray)
		this->complexArray->push_back(complexNumber);
}

const ComplexNumber& ComplexVector::operator[](uint32_t i) const
{
	return (*this->complexArray)[i];
}

ComplexNumber& ComplexVector::operator[](uint32_t i)
{
	return (*this->complexArray)[i];
}

uint64_t ComplexVector::Size() const
{
	return this->complexArray->size();
}

void ComplexVector::Clear()
{
	this->complexArray->clear();
}

void ComplexVector::AddComponent(const ComplexNumber& complexNumber)
{
	this->complexArray->push_back(complexNumber);
}

const std::vector<ComplexNumber>& ComplexVector::GetArray() const
{
	return *this->complexArray;
}

bool ComplexVector::FFT(const ComplexVector& complexVector, bool inverse, Error& error, bool recursing /*= false*/)
{
	this->complexArray->clear();

	if (complexVector.Size() == 0)
	{
		error.Add("Given vector was empty.");
		return false;
	}

	if (complexVector.Size() == 1)
	{
		this->complexArray->push_back(complexVector[0]);
		return true;
	}

	if (complexVector.Size() % 2 != 0)
	{
		// There are varients of the FFT that don't require powers of 2, but...
		error.Add("Expected even vector size.  For the FFT, they should all be powers of 2.");
		return false;
	}

	ComplexVector evenVector, oddVector;

	for (uint32_t i = 0; i < complexVector.Size(); i++)
	{
		const ComplexNumber& coefficient = complexVector[i];
		if (i % 2 == 0)
			evenVector.complexArray->push_back(coefficient);
		else
			oddVector.complexArray->push_back(coefficient);
	}

	ComplexVector evenFFT, oddFFT;

	if (!evenFFT.FFT(evenVector, inverse, error, true))
		return false;

	if (!oddFFT.FFT(oddVector, inverse, error, true))
		return false;

	for (uint32_t i = 0; i < complexVector.Size(); i++)
	{
		ComplexNumber rootOfUnity;
		rootOfUnity.ExpI((inverse ? -1.0 : 1.0) * 2.0 * ADL_PI * double(i) / double(complexVector.Size()));
		uint32_t j = i % (complexVector.Size() / 2);
		ComplexNumber coefficient = evenFFT[j] + rootOfUnity * oddFFT[j];
		if (!recursing && inverse)
			coefficient /= double(complexVector.Size());
		this->complexArray->push_back(coefficient);
	}

	return true;
}