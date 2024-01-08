#pragma once

#include "ByteStream.h"
#include "ByteSwapper.h"

namespace AudioDataLib
{
	class AUDIO_DATA_LIB_API ChunkParser
	{
	public:
		class Chunk;

		ChunkParser();
		virtual ~ChunkParser();

		virtual bool ParseChunkData(BufferStream& inputStream, Chunk* chunk, std::string& error);

		void Clear();
		bool ParseStream(ByteStream& inputStream, std::string& error);
		const Chunk* FindChunk(const std::string& chunkName) const;

		class AUDIO_DATA_LIB_API Chunk
		{
			friend class ChunkParser;

		public:
			Chunk();
			virtual ~Chunk();

			bool ParseStream(BufferStream& inputStream, ChunkParser* chunkParser, std::string& error);
			bool ParseSubChunks(BufferStream& inputStream, ChunkParser* chunkParser, std::string& error);
			const Chunk* FindChunk(const std::string& chunkName) const;

			uint32_t GetNumSubChunks() const { return this->subChunkArray->size(); }
			const std::vector<Chunk*>& GetSubChunkArray() const { return *this->subChunkArray; }

			const uint8_t* GetBuffer() const { return this->buffer; }
			uint32_t GetBufferSize() const { return this->bufferSize; }

			const std::string& GetName() const { return *this->name; }

		protected:
			std::string* name;
			const uint8_t* buffer;
			uint32_t bufferSize;
			std::vector<Chunk*>* subChunkArray;
		};

		ByteSwapper byteSwapper;

	protected:
		uint8_t* buffer;
		uint32_t bufferSize;
		Chunk* rootChunk;
	};
}