#pragma once

#include "Common.h"

namespace AudioDataLib
{
	class AUDIO_DATA_LIB_API ComplexNumber
	{
	public:
		ComplexNumber();
		ComplexNumber(const ComplexNumber& complexNumber);
		ComplexNumber(double realPart, double imagPart);
		virtual ~ComplexNumber();

		void operator=(const ComplexNumber& complexNumber);
		void operator+=(const ComplexNumber& complexNumber);
		void operator-=(const ComplexNumber& complexNumber);
		void operator*=(const ComplexNumber& complexNumber);
		void operator/=(const ComplexNumber& complexNumber);
		void operator+=(double realNumber);
		void operator-=(double realNumber);
		void operator*=(double realNumber);
		void operator/=(double realNumber);
		bool operator==(const ComplexNumber& complexNumber) const;
		bool operator!=(const ComplexNumber& complexNumber) const;
		operator std::string() const;

		void Exp(const ComplexNumber& complexNumber);
		void Exp(double realNumber);
		void ExpI(double realNumber);

		void Log(const ComplexNumber& complexNumber);
		void Log(double realNumber);
		void LogI(double realNumber);

		ComplexNumber Conjugate() const;
		ComplexNumber Inverse() const;

		double SquareMagnitude() const;

		double realPart, imagPart;
	};

	AUDIO_DATA_LIB_API ComplexNumber operator+(const ComplexNumber& complexNumberA, const ComplexNumber& complexNumberB);
	AUDIO_DATA_LIB_API ComplexNumber operator-(const ComplexNumber& complexNumberA, const ComplexNumber& complexNumberB);
	AUDIO_DATA_LIB_API ComplexNumber operator*(const ComplexNumber& complexNumberA, const ComplexNumber& complexNumberB);
	AUDIO_DATA_LIB_API ComplexNumber operator/(const ComplexNumber& complexNumberA, const ComplexNumber& complexNumberB);
	AUDIO_DATA_LIB_API ComplexNumber operator*(const ComplexNumber& complexNumber, double realNumber);
	AUDIO_DATA_LIB_API ComplexNumber operator/(const ComplexNumber& complexNumber, double realNumber);
	AUDIO_DATA_LIB_API ComplexNumber operator*(double realNumber, const ComplexNumber& complexNumber);
	AUDIO_DATA_LIB_API ComplexNumber operator/(double realNumber, const ComplexNumber& complexNumber);
}