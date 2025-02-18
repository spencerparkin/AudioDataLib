#pragma once

#include "AudioDataLib/Common.h"

namespace AudioDataLib
{
	/**
	 * @brief The base class for any kind of data (typically audio data) loaded from a file.
	 * 
	 * Dynamic casts are typically used to decypher what kind of data the user is given when
	 * handed a pointer to this base class.
	 */
	class AUDIO_DATA_LIB_API FileData
	{
	public:
		FileData();
		virtual ~FileData();

		/**
		 * Derivative implimentations should produce informative, human-readable text about the file data.
		 * 
		 * @param[in] fp A pointer to a FILE object to which the text should be written.
		 */
		virtual void DumpInfo(FILE* fp) const = 0;

		/**
		 * Derivative implimentations should produce a command-separated list of human-readable
		 * data which can also be loaded as a CVS in spreadsheet software.
		 * 
		 * @param[in] fp A pointer to a FILE object to which the text should be written.
		 */
		virtual void DumpCSV(FILE* fp) const = 0;

		/**
		 * The derived implimentation should return a new derived instance of the class
		 * in question with identical, but separate data.
		 */
		virtual FileData* Clone() const = 0;
	};
}