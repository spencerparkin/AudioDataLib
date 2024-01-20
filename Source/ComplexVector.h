#pragma once

#include "ComplexNumber.h"

namespace AudioDataLib
{
	class Error;

	class AUDIO_DATA_LIB_API ComplexVector
	{
	public:
		ComplexVector();
		ComplexVector(const ComplexVector& complexVector);
		virtual ~ComplexVector();

		void operator=(const ComplexVector& complexVector);
		const ComplexNumber& operator[](uint32_t i) const;
		ComplexNumber& operator[](uint32_t i);
		uint64_t Size() const;
		void Clear();
		void AddComponent(const ComplexNumber& complexNumber);
		const std::vector<ComplexNumber>& GetArray() const;

		bool FFT(const ComplexVector& complexVector, bool inverse, Error& error, bool recursing = false);

	protected:
		std::vector<ComplexNumber>* complexArray;
	};
}