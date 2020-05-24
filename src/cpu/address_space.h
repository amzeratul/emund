#pragma once

#include <cstdint>
#include <vector>
#include <gsl/span>
#include "../utils/macros.h"
class AddressSpace8BitBy16Bit {
public:
	AddressSpace8BitBy16Bit();
	
	FORCEINLINE uint8_t read(uint16_t address) const
	{
		const auto& segment = memory[address >> 8];
		// TODO: null check?
		return segment[address & 0xFF];
	}
	
	FORCEINLINE void write(uint16_t address, uint8_t value) const
	{
		const auto& segment = memory[address >> 8];
		// TODO: null check?
		segment[address & 0xFF] = value;
	}

	void map(gsl::span<uint8_t> memory, uint16_t startAddress, uint16_t endAddress);
	void unmap(uint16_t startAddress, uint16_t endAddress);

private:
	std::array<uint8_t*, 256> memory;
};
