#pragma once

#include "AudioDataLib/Common.h"
#include <format>

namespace AudioDataLib
{
	/**
	 * A single instance of this class serves as the means of communicating
	 * errors occurring within the AudioDataLib API to the application.
	 * 
	 * Note that I am not a fan of throwing exceptions in C++ code.  My goal
	 * is always to recover the best we can from any error and throwing an
	 * exception is a last resort.
	 */
	class AUDIO_DATA_LIB_API ErrorSystem
	{
	public:
		ErrorSystem();
		virtual ~ErrorSystem();

		static ErrorSystem* Get();

		/**
		 * This is typically called internally by the library to indicate why
		 * an API call returned failure, but the application can call it too,
		 * if they want.
		 * 
		 * @param[in] error This should be a reasonably descritive error message.
		 */
		void Add(const std::string& error);

		/**
		 * The application typically calls this to remove all accumulated errors.
		 * It's best to do this after examining errors, so that you can detect
		 * new errors that may occur later.
		 */
		void Clear();

		/**
		 * Return true here if and only if one or more errors have been accumulated.
		 */
		bool Errors();

		/**
		 * This will aggregate all errors into a single message returned to the caller.
		 */
		std::string GetErrorMessage();

	private:
		std::vector<std::string> errorArray;
	};
}