#pragma once

#include "ComplexNumber.h"

namespace AudioDataLib
{
	class Error;

	/**
	 * @brief These can be imagined as row (or column) vectors having complex-valued elements.
	 * 
	 * A typical use-case is to populate the vector with audio samples, then to perform an FFT.
	 */
	class ComplexVector
	{
	public:
		/**
		 * Construct a new empty vector.
		 */
		ComplexVector();

		/**
		 * Construct a new vector as a copy of the given vector.
		 */
		ComplexVector(const ComplexVector& complexVector);

		virtual ~ComplexVector();

		/**
		 * Remove all complex numbers from this vector, making it empty.
		 */
		void Clear();

		/**
		 * Append a new complex number to the vector.
		 */
		void Add(const ComplexNumber& complexNumber);

		/**
		 * Return the number of elements (complex numbers) in the vector.
		 */
		uint32_t Size() const;

		/**
		 * Assign the given complex vector to this one, making a copy.
		 */
		void operator=(const ComplexVector& complexVector);

		/**
		 * Return the (read-only) complex number at the given offset in this vector.
		 * 
		 * @param[in] i This is the offset of the desired vector.  If it is out of range, your program will crash.
		 */
		const ComplexNumber& operator[](uint32_t i) const;

		/**
		 * Return the complex number at the given offset in this vector.
		 *
		 * @param[in] i This is the offset of the desired vector.  If it is out of range, your program will crash.
		 */
		ComplexNumber& operator[](uint32_t i);

		/**
		 * Perform a Fast Fourier Transform on the given complex vector, placing the result in this complex vector.
		 * 
		 * @param[in] complexVector This is the complex vector to be transformed.
		 * @param[in] inverse If true, an inverse-FFT is performed; a regular FFT otherwise.
		 * @param[out] error This will contain error information if false is returned.
		 * @return True is returned on success; otherwise, false is returned.
		 */
		bool FFT(const ComplexVector& complexVector, bool inverse, Error& error);

	protected:
		std::vector<ComplexNumber>* complexNumberArray;
	};
}