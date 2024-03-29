#pragma once

#include <cstdint>
#include <vector>
#include <gsl/gsl>
#include "../utils/macros.h"

class AddressSpace8BitBy16Bit;

class NESPPU {
public:
	NESPPU();
	
    bool tick();
	
    uint64_t getCycle() const;
	uint32_t getFrameNumber() const;
	uint32_t getX() const;
	uint32_t getY() const;
	bool canGenerateNMI() const;

	void setAddressSpace(AddressSpace8BitBy16Bit& addressSpace);
	void mapRegistersOnCPUAddressSpace(AddressSpace8BitBy16Bit& addressSpace);

	uint8_t readRegister(uint16_t address);
	void writeRegister(uint16_t address, uint8_t value);

	void setFrameBuffer(gsl::span<uint32_t> frameBuffer);
	gsl::span<uint8_t> getOAMData();

private:
	uint64_t cycle = 0;
	uint32_t curX = 0;
	uint32_t curY = 0;
	uint32_t frameN = 0;

	AddressSpace8BitBy16Bit* addressSpace = nullptr;

	// PPU flags
	uint8_t ppuStatus = 0;
	uint8_t ppuCtrl = 0;
	uint8_t ppuMask = 0;

	// PPU registers
	uint16_t vRegister = 0; // Current vram address, 15 bit, 
	uint16_t tRegister = 0; // Temporary vram address, same as above
	uint8_t xRegister = 0; // Fine x scroll, 3 bits
	bool wRegister = false; // Address latch, 1 bit, toggles between 0 and 1, used by $2005 and $2006, reset by $2002

	// Background fetching
	uint16_t patternTableHighShiftRegister;
	uint16_t patternTableLowShiftRegister;
	uint8_t attributeHighShiftRegister;
	uint8_t attributeLowShiftRegister;
	uint8_t nameTableLatch;
	uint8_t attributeLatch;
	uint8_t attributeLatchBit;
	uint8_t patternTableHighLatch;
	uint8_t patternTableLowLatch;

	uint8_t ppuDataBuffer = 0;

	uint8_t oamAddr = 0;

	gsl::span<uint32_t> frameBuffer;
	std::vector<uint8_t> oamData;
	std::vector<uint8_t> oamSecondaryData;

	struct SpriteData {
		uint8_t patternTable0;
		uint8_t patternTable1;
		uint8_t attributes;
		uint8_t x;
	} spriteData[8];

	struct PixelOutput {
		uint8_t value;
		uint8_t palette;
		uint8_t priority;
		uint8_t spriteN;
	};

	void generatePixel(uint8_t x, uint8_t y);
	PixelOutput generateBackground(uint8_t x, uint8_t y);
	PixelOutput generateSprite(uint8_t x, uint8_t y);
	FORCEINLINE uint32_t paletteToColour(uint8_t palette);

	void incrementHorizontalPos();
	void incrementVerticalPos();

	void tickSpriteFetch();
	void tickBackgroundFetch();

	void writeByte(uint16_t address, uint8_t value);
	uint8_t readByte(uint16_t address);
	
	FORCEINLINE bool isRendering() const;
	FORCEINLINE uint8_t reverseBits(uint8_t bits) const;
};
