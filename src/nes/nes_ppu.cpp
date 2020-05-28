#include "nes_ppu.h"

#include "src/cpu/address_space.h"

constexpr static uint8_t PPUCTRL_BASE_NAMETABLE_ADDRESS = 0x03;
constexpr static uint8_t PPUCTRL_VRAM_ADDRESS = 0x4;
constexpr static uint8_t PPUCTRL_SPRITE_PATTERN_TABLE_ADDRESS = 0x8;
constexpr static uint8_t PPUCTRL_BACKGROUND_PATTERN_TABLE_ADDRESS = 0x10;
constexpr static uint8_t PPUCTRL_SPRITE_SIZE = 0x20;
constexpr static uint8_t PPUCTRL_PPU_MASTER_SLAVE_SELECT = 0x40;
constexpr static uint8_t PPUCTRL_GENERATE_NMI = 0x80;

constexpr static uint8_t PPUSTATUS_SPRITE_OVERFLOW = 0x20;
constexpr static uint8_t PPUSTATUS_SPRITE_ZERO_HIT = 0x40;
constexpr static uint8_t PPUSTATUS_VBLANK = 0x80;

constexpr static uint8_t PPUMASK_GREYSCALE = 0x1;
constexpr static uint8_t PPUMASK_SHOW_BACKGROUND_LEFT = 0x2;
constexpr static uint8_t PPUMASK_SHOW_SPRITES_LEFT = 0x4;
constexpr static uint8_t PPUMASK_SHOW_BACKGROUND = 0x8;
constexpr static uint8_t PPUMASK_SHOW_SPRITES = 0x10;
constexpr static uint8_t PPUMASK_EMPHASIZE_RED = 0x20;
constexpr static uint8_t PPUMASK_EMPHASIZE_GREEN = 0x40;
constexpr static uint8_t PPUMASK_EMPHASIZE_BLUE = 0x80;


NESPPU::NESPPU()
{
	ppuStatus = 0;
	oamData.resize(256, 0);
}

bool NESPPU::tick()
{
	const bool isPreRenderLine = curY == 261;
	const bool isPostRenderLine = curY == 240;
	const bool isVisibleLine = curY < 240;

	if (isVisibleLine && curX > 0 && curX <= 256) {
		generatePixel(curX - 1, curY);
	}

	// Not sure if this is right
	if (isVisibleLine || isPreRenderLine) {
		if (curX >= 257 && curX <= 320) {
			oamAddr = 0;
		}
	}

	// Last scanline on odd fields is shorter
	// http://wiki.nesdev.com/w/index.php/PPU_frame_timing
	const uint32_t scanLen = isPreRenderLine && frameN % 2 == 1 ? 340 : 341;

	// Step
	++cycle;
	++curX;	

	if (isPostRenderLine && curX == 1) {
		ppuStatus |= PPUSTATUS_VBLANK;
		return true;
	}

	// Done drawing a scanline
	if (curX == scanLen) {
		curX = 0;
		++curY;

		// Frame done
		if (curY == 262) {
			frameN++;
			curY = 0;
			ppuStatus &= ~PPUSTATUS_VBLANK;
		}
	}
	
	return false;
}

uint32_t NESPPU::getX() const
{
	return curX;
}

uint32_t NESPPU::getY() const
{
	return curY;
}

bool NESPPU::canGenerateNMI() const
{
	return ppuCtrl & PPUCTRL_GENERATE_NMI;
}

void NESPPU::setAddressSpace(AddressSpace8BitBy16Bit& addressSpace)
{
	this->addressSpace = &addressSpace;
}

uint32_t NESPPU::getCycle() const
{
	return cycle;
}

void NESPPU::mapRegistersOnCPUAddressSpace(AddressSpace8BitBy16Bit& addressSpace)
{
	addressSpace.mapRegister(0x2000, 0x3FFF, this, [] (void* self, uint16_t address, uint8_t& value, bool write)
	{
		const uint16_t realAddress = 0x2000 | (address & 0x0F);
		if (write) {
			static_cast<NESPPU*>(self)->writeRegister(realAddress, value);
		} else {
			value = static_cast<NESPPU*>(self)->readRegister(realAddress);
		}
	});
}

uint8_t NESPPU::readRegister(uint16_t address)
{
	switch (address) {
	case 0x2002:
		{
			const uint8_t value = ppuStatus;
			ppuStatus &= ~PPUSTATUS_VBLANK;
			ppuScrollIdx = 0; // Should I do this here?
			ppuAddr = 0;
			return value;
		}
	case 0x2004:
		return oamData[oamAddr];
	case 0x2007:
		{
			const uint8_t value = addressSpace->read(ppuAddr);
			ppuAddr += (ppuCtrl & PPUCTRL_VRAM_ADDRESS) ? 32 : 1;
			return value;
		}
	default:
		throw std::exception();
	}
	return 0;
}

void NESPPU::writeRegister(uint16_t address, uint8_t value)
{
	switch (address) {
	case 0x2000:
		ppuCtrl = value;
		break;
	case 0x2001:
		ppuMask = value;
		break;
	case 0x2003:
		oamAddr = value;
		break;
	case 0x2004:
		if (!isRendering()) {
			oamData[oamAddr++] = value;
		}
		break;
	case 0x2005:
		ppuScroll[ppuScrollIdx] = value;
		ppuScrollIdx = 1 - ppuScrollIdx;
		break;
	case 0x2006:
		if (ppuAddrIdx == 0) {
			ppuAddr = (ppuAddr & 0x00FF) | ((static_cast<uint16_t>(value) << 8) & 0x3F00);
		} else {
			ppuAddr = (ppuAddr & 0xFF00) | value;
		}
		ppuAddrIdx = 1 - ppuAddrIdx;
		break;
	case 0x2007:
		addressSpace->write(ppuAddr, value);
		ppuAddr += (ppuCtrl & PPUCTRL_VRAM_ADDRESS) ? 32 : 1;
		break;
	default:
		throw std::exception();
	}
}

void NESPPU::setFrameBuffer(gsl::span<uint32_t> fb)
{
	frameBuffer = fb;
}

gsl::span<uint8_t> NESPPU::getOAMData()
{
	return oamData;
}

void NESPPU::generatePixel(uint16_t x, uint16_t y)
{
	// Get current tile from nametable
	// TODO: this should be pre-fetched
	const uint16_t nameTableAddress = (ppuCtrl & PPUCTRL_BASE_NAMETABLE_ADDRESS) ? 0x2400 : 0x2000;
	const uint16_t tileX = x >> 3;
	const uint16_t tileY = y >> 3;
	const uint16_t pixelXinTile = x & 0x7;
	const uint16_t pixelYinTile = y & 0x7;
	const uint16_t curTile = addressSpace->read(nameTableAddress + tileX + (tileY << 5));

	// Read tile data
	const uint16_t patternTable = (ppuCtrl & PPUCTRL_BACKGROUND_PATTERN_TABLE_ADDRESS) ? 0x1000 : 0x0000;
	const uint8_t plane0 = addressSpace->read(patternTable + (curTile * 16) + pixelYinTile);
	const uint8_t plane1 = addressSpace->read(patternTable + (curTile * 16) + pixelYinTile + 8);
	const uint8_t tileXBit = 1 << (7 - pixelXinTile);
	const uint8_t value = ((plane0 & tileXBit) != 0 ? 1 : 0) + ((plane1 & tileXBit) != 0 ? 2 : 0);

	// Read attribute
	// TODO: this should also be pre-fetched
	const uint16_t attributeTableAddress = nameTableAddress + 0x3C0;
	const uint16_t attributeEntry = attributeTableAddress + (tileX / 4) + (tileY / 4) * 8;
	const uint8_t attributeTableValue = addressSpace->read(attributeEntry);
	const uint8_t paletteOffset = (tileX & 0x2) | ((tileY & 0x2) << 1);
	const uint8_t paletteEntry = (attributeTableValue >> paletteOffset) & 0x3;

	uint8_t paletteNumber;
	if (value == 0) {
		paletteNumber = addressSpace->read(0x3F00);
	} else {
		paletteNumber = addressSpace->read(0x3F00 + 4 * paletteEntry + value);
	}

	frameBuffer[x + y * 256] = paletteToColour(paletteNumber);
}

uint32_t NESPPU::paletteToColour(uint8_t palette)
{
	const static uint8_t entries[] = {
		84, 84, 84,
		0, 30, 116,
		8, 16, 144,
		48, 0, 136,
		68, 0, 100,
		92, 0, 48,
		84, 4, 0,
		60, 24, 0,
		32, 42, 0,
		8, 58, 0,
		0, 64, 0,
		0, 60, 0,
		0, 50, 60,
		0, 0, 0,
		0, 0, 0,
		0, 0, 0,
		152, 150, 152,
		8, 76, 196,
		48, 50, 236,
		92, 30, 228,
		136, 20, 176,
		160, 20, 100,
		152, 34, 32,
		120, 60, 0,
		84, 90, 0,
		40, 114, 0,
		8, 124, 0,
		0, 118, 40,
		0, 102, 120,
		0, 0, 0,
		0, 0, 0,
		0, 0, 0,
		236, 238, 236,
		76, 154, 236,
		120, 124, 236,
		176, 98, 236,
		228, 84, 236,
		236, 88, 180,
		236, 106, 100,
		212, 136, 32,
		160, 170, 0,
		116, 196, 0,
		76, 208, 32,
		56, 204, 108,
		56, 180, 204,
		60, 60, 60,
		0, 0, 0,
		0, 0, 0,
		236, 238, 236,
		168, 204, 236,
		188, 188, 236,
		212, 178, 236,
		236, 174, 236,
		236, 174, 212,
		236, 180, 176,
		228, 196, 144,
		204, 210, 120,
		180, 222, 120,
		168, 226, 144,
		152, 226, 180,
		160, 214, 228,
		160, 162, 160,
		0, 0, 0,
		0, 0, 0
	};

	return static_cast<uint32_t>(entries[palette * 3]) | (static_cast<uint32_t>(entries[palette * 3 + 1]) << 8) | (static_cast<uint32_t>(entries[palette * 3 + 2]) << 16);
}

bool NESPPU::isRendering() const
{
	return (curY < 240 || curY == 261) && (ppuMask & PPUMASK_SHOW_BACKGROUND || ppuMask & PPUMASK_SHOW_SPRITES);
}
