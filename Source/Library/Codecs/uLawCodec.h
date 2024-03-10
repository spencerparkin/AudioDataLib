#include "Codec.h"

namespace AudioDataLib
{
	/**
	 * @brief Encode or decode audio-data using the standard mu-law specification.
	 */
	class uLawCodec : public Codec
	{
	public:
		uLawCodec();
		virtual ~uLawCodec();

		virtual bool Decode(ByteStream& inputStream, AudioData& audioOut, Error& error) override;
		virtual bool Encode(ByteStream& outputStream, const AudioData& audioIn, Error& error) override;

	private:
		struct ULawTableEntry
		{
			uint8_t rangeCode;
			int16_t minSample, maxSample;
			int16_t intervalSize;
		};

		std::vector<ULawTableEntry>* ulawTableArray;
	};
}