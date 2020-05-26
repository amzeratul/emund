#pragma once

#include <cstdint>
#include <vector>
#include <gsl/gsl>

class AddressSpace8BitBy16Bit;

class NESPPU {
public:
	NESPPU();
	
    bool tick();
	
    uint32_t getCycle() const;
	uint32_t getX() const;
	uint32_t getY() const;

	void mapRegisters(AddressSpace8BitBy16Bit& addressSpace);

	uint8_t readRegister(uint16_t address);
	void writeRegister(uint16_t address, uint8_t value);

	void setFrameBuffer(gsl::span<uint8_t> frameBuffer);

private:
	uint32_t cycle = 0;
	uint32_t curX = 0;
	uint32_t curY = 0;
	uint32_t frameN = 0;

	union PPUStatus {
		struct {
			uint8_t blank : 5;
			bool spriteOverflow : 1;
			bool sprite0Hit : 1;
			bool vBlank : 1;
		};
		uint8_t value;
	} ppuStatus;

	gsl::span<uint8_t> frameBuffer;

	void generatePixel();
};
