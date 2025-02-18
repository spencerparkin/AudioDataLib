#pragma once

#include "AudioDataLib/Common.h"

namespace AudioDataLib
{
	AUDIO_DATA_LIB_API std::string FormatString(const char* format, ...);

	/**
	 * @brief This is class is used throughout the library to communicate errors that can
	 *        occur during the performance of various operations.
	 * 
	 * I'm not a fan of exception-throwing, which some may consider a more modern approach.
	 * It is a bit combersome to pass an Error class reference all over the place, but it's
	 * a convention that I've found works reasonably well, and makes me feel better about the
	 * software cleaning up after itself whether an error occurs or not.  One advantage to
	 * not throwing exceptions is that the caller doesn't have to try (and often fail) to
	 * catch exceptions (due to not knowing which to catch or just not trying).  Rather, the
	 * caller just needs to be in the habit of always checking the return value.  They can
	 * choose to ignore the return value, but it's always there for them to be able to check
	 * if they want.
	 */
	class AUDIO_DATA_LIB_API Error
	{
	public:
		Error();
		virtual ~Error();

		/**
		 * Append an error message to the list of errors maintained by this object.
		 */
		void Add(const std::string& error);

		/**
		 * Return a single string formatted with all errors in the maintained error list.
		 */
		std::string GetErrorMessage() const;

		/**
		 * Return true if one or more errors have been added to the maintained error list.
		 */
		operator bool() const;

		/**
		 * Clear the maintained error list so that it's empty.
		 */
		void Clear();

	protected:
		std::vector<std::string>* errorArray;
	};
}