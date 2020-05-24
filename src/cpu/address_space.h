#pragma once

#include <cstdint>
#include <vector>
#include <gsl/span>

class AddressSpace8BitBy16Bit {
public:
	AddressSpace8BitBy16Bit();
	
	uint8_t read(uint16_t address) const;
	void write(uint16_t address, uint8_t value) const;

	void map(gsl::span<uint8_t> memory, uint16_t baseAddress);
	void unmap(gsl::span<uint8_t> memory, uint16_t baseAddress);

private:
	std::array<uint8_t*, 256> memory;
};
