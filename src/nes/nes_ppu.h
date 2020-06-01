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
	
    uint32_t getCycle() const;
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
	uint32_t cycle = 0;
	uint32_t curX = 0;
	uint32_t curY = 0;
	uint32_t frameN = 0;

	AddressSpace8BitBy16Bit* addressSpace = nullptr;

	uint8_t ppuStatus = 0;
	uint8_t ppuCtrl = 0;
	uint8_t ppuMask = 0;

	uint8_t ppuScroll[2];
	uint8_t ppuScrollIdx = 0;

	uint16_t ppuAddr;
	uint8_t ppuDataBuffer = 0;
	uint8_t ppuAddrIdx = 0;

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

	void tickSpriteFetch();
	void tickBackgroundFetch();

	void writeByte(uint16_t address, uint8_t value);
	uint8_t readByte(uint16_t address);
	
	FORCEINLINE bool isRendering() const;
	FORCEINLINE uint8_t reverseBits(uint8_t bits) const;
};
