#pragma once

#include "AudioDataLib/Common.h"

namespace AudioDataLib
{
	class FileData;
	class ByteStream;

	/**
	 * @brief Derivatives of this class must impliment an interface that can be used to read or write file
	 *        data from or to a given stream.
	 */
	class AUDIO_DATA_LIB_API FileFormat
	{
	public:
		FileFormat();
		virtual ~FileFormat();

		/**
		 * The given file data pointer is assigned an instance of some derivative of the FileData class
		 * which is, in turn, populated with the data found in the given stream.
		 * 
		 * @param[in,out] inputStream This must be a ByteStream derivative that can handle read operations.
		 * @param[out] fileData On success, this pointer is assigned a heap allocation the user is reponsible for freeing.
		 * @return True is returned on success; false otherwise.
		 */
		virtual bool ReadFromStream(ByteStream& inputStream, std::unique_ptr<FileData>& fileData) = 0;

		/**
		 * The given file data is written to the given stream.
		 * 
		 * @param[out] outputStream This must be a ByteStream derivative that can accept write operations.
		 * @param[in] fileData This is the file data to be written to the stream.
		 * @return True is returned on success; false otherwise.
		 */
		virtual bool WriteToStream(ByteStream& outputStream, const FileData* fileData) = 0;

		/**
		 * This is a factory method which will create and return a shared pointer to a FileFormat class
		 * instance derivatve that can handle the given file.
		 * 
		 * @param[in] filePath The extension of the given file is used to determine what FileFormat to return.
		 */
		static std::shared_ptr<FileFormat> CreateForFile(const std::string& filePath);
	};
}