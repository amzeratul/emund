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
	uint8_t ppuAddrIdx = 0;

	uint8_t oamAddr = 0;

	gsl::span<uint32_t> frameBuffer;
	std::vector<uint8_t> oamData;

	void generatePixel(uint16_t x, uint16_t y);
	uint32_t paletteToColour(uint8_t palette);
	
	FORCEINLINE bool isRendering() const;
};
