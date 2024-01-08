#include "ByteSwapper.h"

using namespace AudioDataLib;

ByteSwapper::ByteSwapper()
{
	this->swapsNeeded = false;
}

/*virtual*/ ByteSwapper::~ByteSwapper()
{
}

uint16_t ByteSwapper::Resolve(uint16_t data) const
{
	uint16_t dataResolved = 0;

	if (this->swapsNeeded)
		dataResolved = (data << 8) | (data >> 8);
	else
		dataResolved = data;

	return dataResolved;
}

uint32_t ByteSwapper::Resolve(uint32_t data) const
{
	uint32_t dataResolved = 0;

	if (this->swapsNeeded)
		dataResolved = (data << 24) | ((data & 0x0000FF00) << 8) | ((data & 0x00FF0000) >> 8) | (data >> 24);
	else
		dataResolved = data;

	return dataResolved;
}

uint64_t ByteSwapper::Resolve(uint64_t data) const
{
	uint64_t dataResolved = 0;

	if (this->swapsNeeded)
	{
		dataResolved =
			(data << 56) |
			((0x000000000000FF00ULL) << 42) |
			((0x0000000000FF0000ULL) << 24) |
			((0x00000000FF000000ULL) << 8) |
			((0x000000FF00000000ULL) >> 8) |
			((0x0000FF0000000000ULL) >> 24) |
			((0x00FF000000000000ULL) >> 42) |
			(data >> 56);
	}
	else
		dataResolved = data;

	return dataResolved;
}