#include "ComplexVector.h"
#include "Error.h"

using namespace AudioDataLib;

ComplexVector::ComplexVector()
{
	this->complexNumberArray = new std::vector<ComplexNumber>();
}

ComplexVector::ComplexVector(const ComplexVector& complexVector)
{
	this->complexNumberArray = new std::vector<ComplexNumber>();
	*this = complexVector;
}

/*virtual*/ ComplexVector::~ComplexVector()
{
	delete this->complexNumberArray;
}

void ComplexVector::Clear()
{
	this->complexNumberArray->clear();
}

void ComplexVector::Add(const ComplexNumber& complexNumber)
{
	this->complexNumberArray->push_back(complexNumber);
}

uint32_t ComplexVector::Size() const
{
	return this->complexNumberArray->size();
}

void ComplexVector::operator=(const ComplexVector& complexVector)
{
	this->complexNumberArray->clear();
	for (const ComplexNumber& complexNumber : *complexVector.complexNumberArray)
		this->complexNumberArray->push_back(complexNumber);
}

const ComplexNumber& ComplexVector::operator[](uint32_t i) const
{
	return (*this->complexNumberArray)[i];
}

ComplexNumber& ComplexVector::operator[](uint32_t i)
{
	return (*this->complexNumberArray)[i];
}

// See: Intro to Algorithms by Cormen, Leiserson & Rivest, page 787.
bool ComplexVector::FFT(const ComplexVector& complexVector, bool inverse, Error& error)
{
	this->complexNumberArray->clear();

	if (complexVector.Size() <= 0)
	{
		error.Add("Nothing to transform.");
		return false;
	}

	if (complexVector.Size() == 1)
	{
		this->complexNumberArray->push_back(complexVector[0]);
		return true;
	}

	if (complexVector.Size() % 2 != 0)
	{
		error.Add("Complex vector size not even.  (They should always be a power of 2.)");
		return false;
	}

	ComplexVector evenVector, oddVector;

	for (uint32_t i = 0; i < complexVector.Size(); i++)
	{
		const ComplexNumber& complexNumber = complexVector[i];
		if (i % 2 == 0)
			evenVector.Add(complexNumber);
		else
			oddVector.Add(complexNumber);
	}

	ComplexVector evenFFT, oddFFT;

	if (!evenFFT.FFT(evenVector, inverse, error))
		return false;

	if (!oddFFT.FFT(oddVector, inverse, error))
		return false;

	for (uint32_t i = 0; i < complexVector.Size(); i++)
	{
		ComplexNumber rootOfUnity;
		rootOfUnity.ExpI((inverse ? 1.0 : -1.0) * 2.0 * ADL_PI * double(i) / double(complexVector.Size()));
		uint32_t j = i % (complexVector.Size() / 2);
		ComplexNumber complexNumber = evenFFT[j] + rootOfUnity * oddFFT[j];
		this->Add(complexNumber);
	}

	return true;
}