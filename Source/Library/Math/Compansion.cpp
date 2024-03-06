#include "Compansion.h"

using namespace AudioDataLib;

Compansion::Compansion()
{
	this->ulawTableArray = new std::vector<ULawTableEntry>();

	// See: https://en.wikipedia.org/wiki/M-law_algorithm
	this->ulawTableArray->push_back({ 0x80, 4063, 8158, 256 });
	this->ulawTableArray->push_back({ 0x90, 2015, 4062, 128 });
	this->ulawTableArray->push_back({ 0xA0, 991, 2014, 64 });
	this->ulawTableArray->push_back({ 0xB0, 479, 990, 32 });
	this->ulawTableArray->push_back({ 0xC0, 223, 478, 16 });
	this->ulawTableArray->push_back({ 0xE0, 31, 94, 8 });
	this->ulawTableArray->push_back({ 0xF0, 1, 31, 2 });
	this->ulawTableArray->push_back({ 0x70, -31, -2, 2 });
	this->ulawTableArray->push_back({ 0x60, -95, -32, 4 });
	this->ulawTableArray->push_back({ 0x50, -223, -96, 8 });
	this->ulawTableArray->push_back({ 0x40, -479, -224, 16 });
	this->ulawTableArray->push_back({ 0x30, -991, -480, 32 });
	this->ulawTableArray->push_back({ 0x20, -2015, -992, 64 });
	this->ulawTableArray->push_back({ 0x10, -4063, -2016, 128 });
	this->ulawTableArray->push_back({ 0x00, -8159, -4064, 256 });
}

/*virtual*/ Compansion::~Compansion()
{
	delete this->ulawTableArray;
}

uint8_t Compansion::ULawCompress(int16_t sample)
{
	uint8_t compressedSample = 0;

	if (sample == 0)
		compressedSample = 0xFF;
	else if (sample == -1)
		compressedSample = 0x7F;
	else
	{
		for (const ULawTableEntry& entry : *this->ulawTableArray)
		{
			if (entry.minSample <= sample && sample <= entry.maxSample)
			{
				uint16_t intervalNumber = (sample - entry.minSample) / entry.intervalSize;
				compressedSample = entry.rangeCode | intervalNumber;
				break;
			}
		}
	}

	return compressedSample;
}

int16_t Compansion::ULawExpand(uint8_t sample)
{
	int16_t expandedSample = 0;

	if (sample == 0xFF)
		expandedSample = 0;
	else if (sample == 0x7f)
		expandedSample = -1;
	else
	{
		uint8_t rangeCode = sample & 0xF0;
		int16_t intervalNumber = sample & 0x0F;

		for (const ULawTableEntry& entry : *this->ulawTableArray)
		{
			if (entry.rangeCode == rangeCode)
			{
				expandedSample = entry.minSample + intervalNumber * entry.intervalSize;
				break;
			}
		}
	}

	return expandedSample;
}

uint8_t Compansion::ALawCompress(int16_t sample)
{
	return 0;
}

int16_t Compansion::ALawExpand(uint8_t sample)
{
	return 0;
}