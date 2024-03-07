#pragma once

#include "ByteStream.h"
#include "ByteSwapper.h"

namespace AudioDataLib
{
	class Error;

	/**
	 * @brief This class provides commong RIFF-based parsing support.
	 */
	class AUDIO_DATA_LIB_API ChunkParser
	{
	public:
		class Chunk;

		ChunkParser();
		virtual ~ChunkParser();

		virtual bool ParseChunkData(ReadOnlyBufferStream& inputStream, Chunk* chunk, Error& error);

		void Clear();
		void RegisterSubChunks(const std::string& chunkName);
		bool ParseStream(ByteStream& inputStream, Error& error);
		const Chunk* FindChunk(const std::string& chunkName, const std::string& formType = "", bool caseSensative = true) const;
		void FindAllChunks(const std::string& chunkName, std::vector<const Chunk*>& chunkArray, bool caseSensative = true) const;
		const Chunk* GetRootChunk() const { return this->rootChunk; }

		class AUDIO_DATA_LIB_API Chunk
		{
			friend class ChunkParser;

		public:
			Chunk();
			virtual ~Chunk();

			bool ParseStream(ReadOnlyBufferStream& inputStream, ChunkParser* chunkParser, Error& error);
			bool ParseSubChunks(ReadOnlyBufferStream& inputStream, ChunkParser* chunkParser, Error& error);

			const Chunk* FindChunk(const std::string& chunkName, const std::string& formType, bool caseSensative) const;
			void FindAllChunks(const std::string& chunkName, std::vector<const Chunk*>& chunkArray, bool caseSensative) const;

			bool MatchesName(const std::string& chunkName, bool caseSensative) const;
			bool MatchesFormType(const std::string& formType, bool caseSensative) const;

			uint32_t GetNumSubChunks() const { return (uint32_t)this->subChunkArray->size(); }
			const std::vector<Chunk*>& GetSubChunkArray() const { return *this->subChunkArray; }

			const uint8_t* GetBuffer() const { return this->buffer; }
			uint32_t GetBufferSize() const { return this->bufferSize; }

			const std::string& GetName() const { return *this->name; }
			const std::string& GetFormType() const { return *this->formType; }
			void SetFormType(const char* formType) { *this->formType = formType; }

		protected:
			std::string* name;
			std::string* formType;
			const uint8_t* buffer;
			uint32_t bufferSize;
			std::vector<Chunk*>* subChunkArray;
		};

		ByteSwapper byteSwapper;

	protected:
		uint8_t* buffer;
		uint32_t bufferSize;
		Chunk* rootChunk;
		std::set<std::string>* subChunkSet;
	};
}