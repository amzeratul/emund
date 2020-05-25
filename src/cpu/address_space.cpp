#include "address_space.h"

AddressSpace8BitBy16Bit::AddressSpace8BitBy16Bit()
{
	fallbackPage.fill(0);
	memory.fill(fallbackPage.data());
}

void AddressSpace8BitBy16Bit::map(gsl::span<uint8_t> memoryToMap, uint16_t startAddress, uint16_t endAddress)
{
	Expects(startAddress % pageSize == 0);
	Expects(endAddress % pageSize == pageSize - 1);
	Expects(size_t(endAddress) < pageSize * numPages);

	const size_t dstLen = size_t(endAddress) - startAddress + 1;
	const size_t srcLen = memoryToMap.size();
	const size_t dstPages = dstLen / pageSize;
	const size_t srcPages = srcLen / pageSize;

	Expects(dstLen % srcLen == 0);

	for (size_t pageI = 0; pageI < dstPages; ++pageI) {
		memory[pageI + (startAddress / pageSize)] = memoryToMap.data() + ((pageI % srcPages) * pageSize);
	}
}

void AddressSpace8BitBy16Bit::unmap(uint16_t startAddress, uint16_t endAddress)
{
	Expects(startAddress % pageSize == 0);
	Expects(endAddress % pageSize == 255);
	Expects(size_t(endAddress) < pageSize * numPages);
	
	const size_t len = size_t(endAddress) - startAddress + 1;

	for (size_t pageI = 0; pageI < (len / pageSize); ++pageI) {
		memory[pageI + (startAddress / pageSize)] = fallbackPage.data();
	}
}
