#include "AudioDataLib/Codecs/uLawCodec.h"
#include "AudioDataLib/ByteStream.h"
#include "AudioDataLib/ErrorSystem.h"

using namespace AudioDataLib;

uLawCodec::uLawCodec()
{
	// See: https://en.wikipedia.org/wiki/M-law_algorithm
	this->ulawTableArray.push_back({ 0x80, 4063, 8158, 256 });
	this->ulawTableArray.push_back({ 0x90, 2015, 4062, 128 });
	this->ulawTableArray.push_back({ 0xA0, 991, 2014, 64 });
	this->ulawTableArray.push_back({ 0xB0, 479, 990, 32 });
	this->ulawTableArray.push_back({ 0xC0, 223, 478, 16 });
	this->ulawTableArray.push_back({ 0xD0, 95, 222, 8 });
	this->ulawTableArray.push_back({ 0xE0, 31, 94, 4 });
	this->ulawTableArray.push_back({ 0xF0, 1, 31, 2 });
	this->ulawTableArray.push_back({ 0x70, -31, -2, 2 });
	this->ulawTableArray.push_back({ 0x60, -95, -32, 4 });
	this->ulawTableArray.push_back({ 0x50, -223, -96, 8 });
	this->ulawTableArray.push_back({ 0x40, -479, -224, 16 });
	this->ulawTableArray.push_back({ 0x30, -991, -480, 32 });
	this->ulawTableArray.push_back({ 0x20, -2015, -992, 64 });
	this->ulawTableArray.push_back({ 0x10, -4063, -2016, 128 });
	this->ulawTableArray.push_back({ 0x00, -8159, -4064, 256 });
}

/*virtual*/ uLawCodec::~uLawCodec()
{
}

/*virtual*/ bool uLawCodec::Decode(ByteStream& inputStream, AudioData& audioOut)
{
	if (audioOut.GetFormat().bitsPerSample != 16)
	{
		ErrorSystem::Get()->Add("Expected audio format to be 16-bit.");
		return false;
	}

	if (audioOut.GetFormat().sampleType != AudioData::Format::SIGNED_INTEGER)
	{
		ErrorSystem::Get()->Add("Expected audio format to be signed-integer.");
		return false;
	}

	audioOut.SetAudioBufferSize(inputStream.GetSize() * 2);
	
	uint16_t* expandedSample = reinterpret_cast<uint16_t*>(audioOut.GetAudioBuffer());

	while(inputStream.GetSize() > 0)
	{
		uint8_t sample = 0;
		if (1 != inputStream.ReadBytesFromStream(&sample, 1))
		{
			ErrorSystem::Get()->Add("Failed to read compressed sample from the given input stream.");
			return false;
		}

		if (sample == 0xFF)
			*expandedSample = 0;
		else if (sample == 0x7f)
			*expandedSample = -1;
		else
		{
			*expandedSample = 0;

			uint8_t rangeCode = sample & 0xF0;
			int16_t intervalNumber = sample & 0x0F;

			bool foundRangeCode = false;
			double sampleResult = 0.0;
			for (const ULawTableEntry& entry : this->ulawTableArray)
			{
				if (entry.rangeCode == rangeCode)
				{
					sampleResult = double(entry.minSample + (entry.minSample < 0 ? -1 : 1) * intervalNumber * entry.intervalSize);
					foundRangeCode = true;
					break;
				}
			}

			if (!foundRangeCode)
			{
				ErrorSystem::Get()->Add(std::format("Failed to find range code {} in u-law table.", rangeCode));
				return false;
			}

			constexpr double numeric_limits_int14 = 8159.0;
			constexpr double numeric_limits_int16 = double(std::numeric_limits<int16_t>::max());
			constexpr double conversionFactor = numeric_limits_int16 / numeric_limits_int14;
			
			sampleResult *= conversionFactor;
			sampleResult = ADL_CLAMP(sampleResult, -numeric_limits_int16, numeric_limits_int16);
			*expandedSample = int16_t(sampleResult);
		}

		expandedSample++;
	}

	return true;
}

/*virtual*/ bool uLawCodec::Encode(ByteStream& outputStream, const AudioData& audioIn)
{
	if (audioIn.GetFormat().bitsPerSample != 16)
	{
		ErrorSystem::Get()->Add("Expected audio format to be 16-bit.");
		return false;
	}

	if (audioIn.GetFormat().sampleType != AudioData::Format::SIGNED_INTEGER)
	{
		ErrorSystem::Get()->Add("Expected audio format to be signed-integer.");
		return false;
	}

	if (audioIn.GetAudioBufferSize() % 2 != 0)
	{
		ErrorSystem::Get()->Add(std::format("Expected audio buffer size {} to be divisible by two.", audioIn.GetAudioBufferSize()));
		return false;
	}

	const uint16_t* sampleBuffer = reinterpret_cast<const uint16_t*>(audioIn.GetAudioBuffer());

	for (uint64_t i = 0; i < audioIn.GetAudioBufferSize() / 2; i++)
	{
		uint16_t sample = sampleBuffer[i];

		uint8_t compressedSample = 0;

		if (sample == 0)
			compressedSample = 0xFF;
		else if (sample == -1)
			compressedSample = 0x7F;
		else
		{
			compressedSample = 0xFF;

			for (const ULawTableEntry& entry : this->ulawTableArray)
			{
				if (entry.minSample <= sample && sample <= entry.maxSample)
				{
					uint16_t intervalNumber = (sample - entry.minSample) / entry.intervalSize;
					compressedSample = entry.rangeCode | intervalNumber;
					break;
				}
			}

			if (compressedSample == 0xFF)
			{
				ErrorSystem::Get()->Add("Failed to located sample in u-law table.");
				return false;
			}
		}

		if (1 != outputStream.WriteBytesToStream(&compressedSample, 1))
		{
			ErrorSystem::Get()->Add("Failed to write compressed audio byte to output stream.");
			return false;
		}
	}

	return true;
}