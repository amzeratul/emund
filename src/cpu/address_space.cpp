#include "address_space.h"

#include <halley.hpp>
using namespace Halley;

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

void AddressSpace8BitBy16Bit::mapRegister(uint16_t startAddress, uint16_t endAddress, void* data, RegisterCallback callback)
{
	for (auto& r: registers) {
		if (!r.callback) {
			r.data = data;
			r.callback = callback;
			r.startAddress = startAddress;
			r.endAddress = endAddress;

			registersStartAddress = std::min(registersStartAddress, startAddress);
			registersEndAddress = std::max(registersEndAddress, endAddress);
			return;
		}
	}

	// All slots allocated
}

void AddressSpace8BitBy16Bit::dump(uint16_t startAddress, uint16_t endAddress)
{
	Expects(startAddress % 16 == 0);
	Expects(endAddress % 16 == 15);
	
	// Memory dump
	constexpr size_t bytesPerRow = 16;
	Halley::Logger::logDev("\nRAM dump:");
	Logger::logDev("      00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F");
	for (size_t i = startAddress; i < endAddress; i += bytesPerRow) {
		String row = toString(i, 16, 4) + ": ";

		const uint8_t* src = memory[i >> 8] + (i & 0xFF);
		
		for (size_t j = 0; j < bytesPerRow; ++j) {
			row += toString(uint32_t(src[j]), 16, 2).asciiUpper() + " ";
		}

		for (size_t j = 0; j < bytesPerRow; ++j) {
			row += src[j] >= 32 ? static_cast<char>(src[j]) : '.';
		}
		
		Logger::logDev(row);
	}
}
