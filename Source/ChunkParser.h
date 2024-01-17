#pragma once

#include "ByteStream.h"
#include "ByteSwapper.h"

namespace AudioDataLib
{
	class Error;

	class AUDIO_DATA_LIB_API ChunkParser
	{
	public:
		class Chunk;

		ChunkParser();
		virtual ~ChunkParser();

		virtual bool ParseChunkData(ReadOnlyBufferStream& inputStream, Chunk* chunk, Error& error);

		void Clear();
		bool ParseStream(ByteStream& inputStream, Error& error);
		const Chunk* FindChunk(const std::string& chunkName, bool caseSensative = true) const;
		void FindAllChunks(const std::string& chunkName, std::vector<const Chunk*>& chunkArray, bool caseSensative = true) const;

		class AUDIO_DATA_LIB_API Chunk
		{
			friend class ChunkParser;

		public:
			Chunk();
			virtual ~Chunk();

			bool ParseStream(ReadOnlyBufferStream& inputStream, ChunkParser* chunkParser, Error& error);
			bool ParseSubChunks(ReadOnlyBufferStream& inputStream, ChunkParser* chunkParser, Error& error);

			const Chunk* FindChunk(const std::string& chunkName, bool caseSensative) const;
			void FindAllChunks(const std::string& chunkName, std::vector<const Chunk*>& chunkArray, bool caseSensative) const;

			bool MatchesName(const std::string& chunkName, bool caseSensative) const;

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