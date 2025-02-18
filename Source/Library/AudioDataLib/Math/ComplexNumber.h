#pragma once

#include "AudioDataLib/Common.h"

namespace AudioDataLib
{
	/**
	 * @brief Instances of this class represent complex numbers in the complex plane.
	 * 
	 * Interestingly, complex numbers show up in the analysis of wave-forms; particularly
	 * when an FFT (fast forier transform) is performed.
	 */
	class AUDIO_DATA_LIB_API ComplexNumber
	{
	public:
		/**
		 * The default constructor always initializes this complex number to zero.
		 */
		ComplexNumber();

		/**
		 * Initialize this complex number as a copy of the given complex number.
		 */
		ComplexNumber(const ComplexNumber& complexNumber);

		/**
		 * Initialize this complex number with the given components.
		 */
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

		/**
		 * Make this complex number the base of the natural logarithm raised to the power of the given complex number.
		 */
		void Exp(const ComplexNumber& complexNumber);

		/**
		 * Make this complex number the base of the natural logarithm raised to the power of the given real number.
		 */
		void Exp(double realNumber);

		/**
		 * Make this complex number the base of the natural logarithm raised to the power of the given imaginary number (or real multiple of the imaginary unit.)
		 */
		void ExpI(double realNumber);

		/**
		 * Make this complex number the natural logarithm of the given complex number.
		 */
		void Log(const ComplexNumber& complexNumber);

		/**
		 * Make this complex number the natural logarithm of the given real number.
		 */
		void Log(double realNumber);

		/**
		 * Make this complex number the natural logarithm of the given imaginary number (or real multiple of the imaginary unit.)
		 */
		void LogI(double realNumber);

		/**
		 * Calculate and return the conjugate of this complex number.
		 */
		ComplexNumber Conjugate() const;

		/**
		 * Calculate and return the multiplicative inverse of this complex number.
		 */
		ComplexNumber Inverse() const;

		/**
		 * Calculate and return the square magnitude of this complex number.
		 */
		double SquareMagnitude() const;

		/**
		 * Calculate and return the magnitude of this complex number.
		 */
		double Magnitude() const;

		/**
		 * Calculate and return the angle (in [0,2pi)) made by this complex number when compared to the +X axis.
		 */
		double Angle() const;

		double realPart;		///< The real component of this complex number.
		double imagPart;		///< The imaginary component (or real multiple of the imaginary unit) of this complex number.
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