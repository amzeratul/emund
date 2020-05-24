#include "address_space.h"

AddressSpace8BitBy16Bit::AddressSpace8BitBy16Bit()
{
	memory.fill(nullptr);
}

uint8_t AddressSpace8BitBy16Bit::read(uint16_t address) const
{
	const auto& segment = memory[address >> 8];
	// TODO: null check?
	return segment[address & 0xFF];
}

void AddressSpace8BitBy16Bit::write(uint16_t address, uint8_t value) const
{
	const auto& segment = memory[address >> 8];
	// TODO: null check?
	segment[address & 0xFF] = value;
}

void AddressSpace8BitBy16Bit::map(gsl::span<uint8_t> memoryToMap, uint16_t startAddress, uint16_t endAddress)
{
	constexpr size_t segmentSize = 256;
	
	Expects(startAddress % segmentSize == 0);
	Expects(endAddress % segmentSize == 255);

	const size_t dstLen = size_t(endAddress) - startAddress + 1;
	const size_t srcLen = memoryToMap.size();
	const size_t dstSegments = dstLen / segmentSize;
	const size_t srcSegments = srcLen / segmentSize;

	Expects(dstLen % srcLen == 0);

	for (size_t i = 0; i < dstSegments; ++i) {
		memory[i + (startAddress / segmentSize)] = memoryToMap.data() + ((i % srcSegments) * segmentSize);
	}
}

void AddressSpace8BitBy16Bit::unmap(uint16_t startAddress, uint16_t endAddress)
{
	constexpr size_t segmentSize = 256;
	
	Expects(startAddress % segmentSize == 0);
	Expects(endAddress % segmentSize == 255);
	const size_t len = size_t(endAddress) - startAddress + 1;

	for (size_t i = 0; i < (len / segmentSize); ++i) {
		memory[i + (startAddress / segmentSize)] = nullptr;
	}
}
