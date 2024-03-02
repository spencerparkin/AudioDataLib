#pragma once

#include "FileFormat.h"
#include "ChunkParser.h"

namespace AudioDataLib
{
	class Error;

	/**
	 * @brief This class knows how to read (and one day write) DLS files.
	 *
	 * DLS stands for downloadable sound.  It's a RIFF-based file that contains a bunch
	 * of instruments (timbers, you could say) and their associated sound samples, and
	 * details how those sounds are to be replayed as a function of the MIDI protocol.
	 */
	class AUDIO_DATA_LIB_API DownloadableSoundFormat : public FileFormat
	{
	public:
		DownloadableSoundFormat();
		virtual ~DownloadableSoundFormat();

		virtual bool ReadFromStream(ByteStream& inputStream, FileData*& fileData, Error& error) override;
		virtual bool WriteToStream(ByteStream& outputStream, const FileData* fileData, Error& error) override;
	};
}