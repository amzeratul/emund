#pragma once

#include <cstdint>

class AddressSpace8BitBy16Bit;

class NESPPU {
public:
	NESPPU();
	
    void tick();
    uint32_t getCycle();

	void mapRegisters(AddressSpace8BitBy16Bit& addressSpace);

	uint8_t readRegister(uint16_t address);
	void writeRegister(uint16_t address, uint8_t value);

	void startVBlank();

private:
	uint32_t cycle = 0;

	union PPUStatus {
		struct {
			uint8_t blank : 5;
			bool spriteOverflow : 1;
			bool sprite0Hit : 1;
			bool vBlank : 1;
		};
		uint8_t value;
	} ppuStatus;
};
