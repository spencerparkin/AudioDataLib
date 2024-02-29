#pragma once

#include "Common.h"
#include "AudioData.h"

namespace AudioDataLib
{
	class Mutex;

	/**
	 * @brief This is the base class for all types of byte streams used in the library.
	 *        
	 * Some derivatives are read-only, write-only, or read-write.  Consequently, not all
	 * interface methods are supported by all derivatives.
	 */
	class AUDIO_DATA_LIB_API ByteStream
	{
	public:
		ByteStream();
		virtual ~ByteStream();

		/**
		 * Write the given buffer to this stream.
		 * 
		 * @param[in] buffer This is a pointer to the array of bytes to write to the stream.
		 * @param[in] bufferSize This is the size of the given array in bytes.
		 * @return The number of bytes actually written to the stream is returned.
		 */
		virtual uint64_t WriteBytesToStream(const uint8_t* buffer, uint64_t bufferSize) = 0;

		/**
		 * Read from this stream into the given buffer.
		 * 
		 * @param[out] buffer This is a pointer to an array where bytes read from the stream will be written.
		 * @param[in] bufferSize This is the size in bytes of the given array.
		 * @return The number of bytes actually read from the stream and written to the given buffer is returned.
		 */
		virtual uint64_t ReadBytesFromStream(uint8_t* buffer, uint64_t bufferSize) = 0;

		/**
		 * Read from this stream into the given buffer, but unlike ReadBytesFromStream, do not modify the stream.
		 * That is, a subsequent read from the stream will read from the same location.
		 * 
		 * @param[out] buffer This is a pointer to an array where bytes read from the stream will be written.
		 * @param[in] bufferSize This is the size in bytes of the given array.
		 * @return The number of bytes actually read from the stream and written to the given buffer is returned.
		 */
		virtual uint64_t PeekBytesFromStream(uint8_t* buffer, uint64_t bufferSize);

		/**
		 * Return the amount of data (in bytes) currently stored in the stream.
		 */
		virtual uint64_t GetSize() const = 0;

		/**
		 * Indicate whether there is data available to be read from the stream.
		 */
		virtual bool CanRead() = 0;

		/**
		 * Indicate whether it is possible to write additional data to the stream.
		 */
		virtual bool CanWrite() = 0;

		/**
		 * Read an amount of data from the stream equal to the size of the given type.
		 * Byte-swapping may be a consideration here.  Note that a failed read could
		 * still be a partial read.  In other words, the stream can get modified even
		 * if the read fails.
		 */
		template<typename T>
		bool ReadType(T* value)
		{
			uint64_t numBytesRead = this->ReadBytesFromStream((uint8_t*)value, sizeof(T));
			return numBytesRead == sizeof(T);
		}

		/**
		 * Write an amount of data to the stream equal to the size of the given type.
		 * Byte-swapping may be a consideration here.  Note that a failed write could
		 * still be a partial write.  In other words, the stream can get modified even
		 * if the write fails.
		 */
		template<typename T>
		bool WriteType(const T* value)
		{
			uint64_t numBytesWritten = this->WriteBytesToStream((const uint8_t*)value, sizeof(T));
			return numBytesWritten == sizeof(T);
		}
	};

	/**
	 * @brief This is the base class for all file-based streams.
	 */
	class AUDIO_DATA_LIB_API FileStream : public ByteStream
	{
	public:
		FileStream(const char* filePath, const char* mode);
		virtual ~FileStream();

		bool IsOpen();
		void Close();

	protected:
		FILE* fp;
	};

	/**
	 * @brief This is a file stream that can be used to read bytes from a file.
	 */
	class AUDIO_DATA_LIB_API FileInputStream : public FileStream
	{
	public:
		FileInputStream(const char* filePath);
		virtual ~FileInputStream();

		virtual uint64_t WriteBytesToStream(const uint8_t* buffer, uint64_t bufferSize) override;
		virtual uint64_t ReadBytesFromStream(uint8_t* buffer, uint64_t bufferSize) override;

		virtual uint64_t GetSize() const override;

		virtual bool CanRead() override;
		virtual bool CanWrite() override;
	};

	/**
	 * @brief This is a file stream that can be used to write bytes to a file.
	 */
	class AUDIO_DATA_LIB_API FileOutputStream : public FileStream
	{
	public:
		FileOutputStream(const char* filePath);
		virtual ~FileOutputStream();

		virtual uint64_t WriteBytesToStream(const uint8_t* buffer, uint64_t bufferSize) override;
		virtual uint64_t ReadBytesFromStream(uint8_t* buffer, uint64_t bufferSize) override;

		virtual uint64_t GetSize() const override;

		virtual bool CanRead() override;
		virtual bool CanWrite() override;
	};

	/**
	 * @brief This is a stream that can be used to read bytes from any location in memory.
	 */
	class AUDIO_DATA_LIB_API ReadOnlyBufferStream : public ByteStream
	{
	public:
		ReadOnlyBufferStream(const uint8_t* buffer, uint64_t bufferSize);
		virtual ~ReadOnlyBufferStream();

		virtual uint64_t WriteBytesToStream(const uint8_t* buffer, uint64_t bufferSize) override;
		virtual uint64_t ReadBytesFromStream(uint8_t* buffer, uint64_t bufferSize) override;

		virtual uint64_t PeekBytesFromStream(uint8_t* buffer, uint64_t bufferSize) override;

		virtual uint64_t GetSize() const override;

		virtual bool CanRead() override;
		virtual bool CanWrite() override;

		uint64_t GetReadOffset() const { return this->readOffset; }
		bool SetReadOffset(uint64_t readOffset);

		const uint8_t* GetBuffer() const { return this->readOnlyBuffer; }

		void Reset();

	protected:
		const uint8_t* readOnlyBuffer;
		uint64_t readOnlyBufferSize;
		uint64_t readOffset;
	};

	/**
	 * @brief This is stream that can be used to write bytes to any location in memory.
	 */
	class AUDIO_DATA_LIB_API WriteOnlyBufferStream : public ByteStream
	{
	public:
		WriteOnlyBufferStream(uint8_t* buffer, uint64_t bufferSize);
		virtual ~WriteOnlyBufferStream();

		virtual uint64_t WriteBytesToStream(const uint8_t* buffer, uint64_t bufferSize) override;
		virtual uint64_t ReadBytesFromStream(uint8_t* buffer, uint64_t bufferSize) override;

		virtual uint64_t GetSize() const override;

		virtual bool CanRead() override;
		virtual bool CanWrite() override;

		uint64_t GetWriteOffset() const { return this->writeOffset; }
		bool SetWriteOffset(uint64_t readOffset);

		const uint8_t* GetBuffer() const { return this->writeOnlyBuffer; }

		void Reset();

	protected:
		uint8_t* writeOnlyBuffer;
		uint64_t writeOnlyBufferSize;
		uint64_t writeOffset;
	};

	/**
	 * @brief This stream is designed to act as a FIFO queue of audio data of a particular format.
	 * 
	 * This class by itself often acts, single-handedly, as the glue between
	 * this library and one that interfaces with the actual audio hardware.
	 * The low-level library will dicate the stream format, and then this library
	 * will accommodate that format.  The low-level library reads from (or writes to)
	 * this stream while this library writes to (or reads from) it.
	 */
	class AUDIO_DATA_LIB_API AudioStream : public ByteStream
	{
	public:
		AudioStream();
		AudioStream(const AudioData* audioData);
		virtual ~AudioStream();

		virtual uint64_t WriteBytesToStream(const uint8_t* buffer, uint64_t bufferSize) override;
		virtual uint64_t ReadBytesFromStream(uint8_t* buffer, uint64_t bufferSize) override;

		virtual uint64_t GetSize() const override;

		virtual bool CanRead() override;
		virtual bool CanWrite() override;

		const AudioData::Format& GetFormat() const { return this->format; }
		void SetFormat(const AudioData::Format& format) { this->format = format; }

	protected:
		AudioData::Format format;
		ByteStream* byteStream;
	};

	/**
	 * @brief This is a thread-safe version of the AudioStream class.
	 * 
	 * As such, it is a good candidate for connecting any AudioDataLib class that produces or consumes audio
	 * to some other API that wants to consume or produce audio, respectively.  For example, an audio callback
	 * (which typically runs on its own time-sensative thread) would read from such a stream while the main
	 * thread can write to the stream.
	 */
	class AUDIO_DATA_LIB_API ThreadSafeAudioStream : public AudioStream
	{
	public:
		ThreadSafeAudioStream(std::shared_ptr<Mutex> mutex);
		virtual ~ThreadSafeAudioStream();

		virtual uint64_t WriteBytesToStream(const uint8_t* buffer, uint64_t bufferSize) override;
		virtual uint64_t ReadBytesFromStream(uint8_t* buffer, uint64_t bufferSize) override;

		virtual uint64_t GetSize() const override;

		virtual bool CanRead() override;
		virtual bool CanWrite() override;

	protected:
		// I would prefer not to own a pointer to a shared pointer here, but it doesn't work when trying to make a DLL.
		std::shared_ptr<Mutex>* mutex;
	};

	/**
	 * @brief This is a read/write, in-memory stream of bytes that is bounded only by the memory limitations of the operating system.
	 */
	class AUDIO_DATA_LIB_API MemoryStream : public ByteStream
	{
	public:
		MemoryStream();
		virtual ~MemoryStream();

		virtual uint64_t WriteBytesToStream(const uint8_t* buffer, uint64_t bufferSize) override;
		virtual uint64_t ReadBytesFromStream(uint8_t* buffer, uint64_t bufferSize) override;

		virtual uint64_t GetSize() const override;

		virtual bool CanRead() override;
		virtual bool CanWrite() override;

		void Clear();

	protected:
		class Chunk
		{
		public:
			Chunk(uint64_t bufferSize);
			virtual ~Chunk();

			uint64_t WriteToChunk(const uint8_t* givenBuffer, uint64_t givenBufferSize);
			uint64_t ReadFromChunk(uint8_t* givenBuffer, uint64_t givenBufferSize);

			uint64_t GetSize() const;

			uint8_t* buffer;
			uint64_t bufferSize;
			uint64_t startOffset;
			uint64_t endOffset;
		};

		std::list<Chunk*>* chunkList;
		uint64_t chunkSize;
	};
}