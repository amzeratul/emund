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

	union PPUCTRL {
		enum class MasterSlave {
			ReadFromExt,
			OutputColourOnExt
		};

		enum class SpriteSize {
			Size8x8,
			Size8x16
		};

		enum class AddressIncrement {
			Horizontal,
			Vertical
		};

		enum class DrawPatternAddress {
			Pattern0000,
			Pattern1000
		};

		struct {
			uint8_t nametableAddress : 2;
			AddressIncrement vramAddressIncrement : 1;
			DrawPatternAddress spritePattern : 1;
			DrawPatternAddress backgroundPattern : 1;
			SpriteSize spriteSize : 1;
			MasterSlave ppuMasterSlave : 1;
			bool generateNMI : 1;
		};
		uint8_t value;
	} ppuCtrl;

	union PPUMask {
		struct {
			bool greyscale : 1;
			bool showLeftBackground : 1;
			bool showLeftSprites : 1;
			bool showBackground : 1;
			bool showSprites : 1;
			bool emphasizeRed : 1;
			bool emphasizeGreen : 1;
			bool emphasizeBlue : 1;
		};
		uint8_t value;
	} ppuMask;

	uint8_t ppuScroll[2];
	uint8_t ppuScrollIdx = 0;

	uint16_t ppuAddr;
	uint8_t ppuAddrIdx = 0;

	gsl::span<uint8_t> frameBuffer;

	void generatePixel();
};
