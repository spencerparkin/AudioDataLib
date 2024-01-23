#pragma once

#include "ComplexNumber.h"

namespace AudioDataLib
{
	class Error;

	class ComplexVector
	{
	public:
		ComplexVector();
		ComplexVector(const ComplexVector& complexVector);
		virtual ~ComplexVector();

		void Clear();
		void Add(const ComplexNumber& complexNumber);
		uint32_t Size() const;
		void operator=(const ComplexVector& complexVector);
		const ComplexNumber& operator[](uint32_t i) const;
		ComplexNumber& operator[](uint32_t i);

		bool FFT(const ComplexVector& complexVector, bool inverse, Error& error);

	protected:
		std::vector<ComplexNumber>* complexNumberArray;
	};
}