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

void AddressSpace8BitBy16Bit::map(gsl::span<uint8_t> memoryToMap, uint16_t baseAddress)
{
	Expects(memoryToMap.size() % 256 == 0);
	Expects(baseAddress % 256 == 0);
	const size_t len = memoryToMap.size();

	for (size_t i = 0; i < (len / 256); ++i) {
		memory[i + (baseAddress / 256)] = memoryToMap.data() + (i * 256);
	}
}

void AddressSpace8BitBy16Bit::unmap(gsl::span<uint8_t> memoryToUnmap, uint16_t baseAddress)
{
	Expects(memoryToUnmap.size() % 256 == 0);
	Expects(baseAddress % 256 == 0);
	const size_t len = memoryToUnmap.size();

	for (size_t i = 0; i < (len / 256); ++i) {
		memory[i + (baseAddress / 256)] = nullptr;
	}
}
