#pragma once

#include <cstdint>
#include <vector>
#include <gsl/span>
#include "../utils/macros.h"

class AddressSpace8BitBy16Bit {
public:
	using RegisterCallback = void(*)(void*, uint16_t, uint8_t&, bool);

	AddressSpace8BitBy16Bit();
	
	FORCEINLINE uint8_t read(uint16_t address) const
	{
		if (address >= registersStartAddress && address <= registersEndAddress) {
			for (auto& r: registers) {
				if (address >= r.startAddress && address <= r.endAddress) {
					uint8_t value = 0;
					r.callback(r.data, address, value, false);
					return value;
				}
			}
		}
		
		const auto& page = memory[address >> 8];
		return page[address & 0xFF];
	}
	
	FORCEINLINE void write(uint16_t address, uint8_t value) const
	{
		if (address >= registersStartAddress && address <= registersEndAddress) {
			for (auto& r: registers) {
				if (address >= r.startAddress && address <= r.endAddress) {
					r.callback(r.data, address, value, true);
					return;
				}
			}
		}

		const auto& page = memory[address >> 8];
		page[address & 0xFF] = value;
	}

	void map(gsl::span<uint8_t> memory, uint16_t startAddress, uint16_t endAddress);
	void unmap(uint16_t startAddress, uint16_t endAddress);

	void mapRegister(uint16_t startAddress, uint16_t endAddress, void* data, RegisterCallback callback);

	void dump(uint16_t startAddress, uint16_t endAddress);

private:
	constexpr static size_t pageSize = 256;
	constexpr static size_t numPages = 256;

	std::array<uint8_t*, numPages> memory;
	std::array<uint8_t, pageSize> fallbackPage;

	uint16_t registersStartAddress = 0xFFFF;
	uint16_t registersEndAddress = 0x0000;
	struct Register {
		uint16_t startAddress = 0xFFFF;
		uint16_t endAddress = 0xFFFF;
		RegisterCallback callback = nullptr;
		void* data = nullptr;
	};
	std::array<Register, 4> registers;
};
