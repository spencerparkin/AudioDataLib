#include "AudioDataLib/Codecs/Codec.h"

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

		virtual bool Decode(ByteStream& inputStream, AudioData& audioOut) override;
		virtual bool Encode(ByteStream& outputStream, const AudioData& audioIn) override;

		// TODO: Add compression parameter here.
	};
}