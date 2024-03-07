#include "Codec.h"

namespace AudioDataLib
{
	/**
	 * @brief Encode or decode audio-data using the standard A-law specification.
	 */
	class ALawCodec : public Codec
	{
	public:
		ALawCodec();
		virtual ~ALawCodec();

		virtual bool Decode(ByteStream& inputStream, AudioData& audioOut, Error& error) override;
		virtual bool Encode(ByteStream& outputStream, const AudioData& audioIn, Error& error) override;

		// TODO: Add compression parameter here.
	};
}