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
	oamSecondaryData.resize(32, 0);
}

bool NESPPU::tick()
{
	const bool isPreRenderLine = curY == 261;
	const bool isPostRenderLine = curY == 240;
	const bool isVisibleLine = curY < 240;

	if (isVisibleLine && curX > 0 && curX <= 256) {
		generatePixel(curX - 1, curY);
	}

	if (isVisibleLine) {
		tickSpriteFetch();
		tickBackgroundFetch();
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
		if (curY < 240 && curX >= 1 && curX <= 64) {
			// On cycles 1-64 of a visible scanline (when it's initializing secondary OAM), this always returns 0xFF
			return 0xFF;
		}
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

void NESPPU::generatePixel(uint8_t x, uint8_t y)
{
	auto bg = generateBackground(x, y);
	auto sprite = generateSprite(x, y);

	PixelOutput result;
	if (sprite.value != 0 && bg.value != 0) {
		result = sprite.priority == 0 ? sprite : bg;
	} else if (sprite.value != 0) {
		result = sprite;
	} else if (bg.value != 0) {
		result = bg;
	} else {
		result = { bg.value, 0 };
	}
	
	const uint8_t colour = addressSpace->readDirect(0x3F00 + 4 * result.palette + result.value);

	frameBuffer[size_t(x) + size_t(y) * 256] = paletteToColour(colour);
}

NESPPU::PixelOutput NESPPU::generateBackground(uint8_t x, uint8_t y)
{
	// Get current tile from nametable
	// TODO: this should be pre-fetched
	const uint16_t nameTableAddress = (ppuCtrl & PPUCTRL_BASE_NAMETABLE_ADDRESS) ? 0x2400 : 0x2000;
	const uint8_t tileX = x >> 3;
	const uint8_t tileY = y >> 3;
	const uint8_t pixelXinTile = x & 0x7;
	const uint8_t pixelYinTile = y & 0x7;
	const uint8_t curTile = addressSpace->readDirect(nameTableAddress + tileX + (tileY << 5));

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
	const uint8_t attributeTableValue = addressSpace->readDirect(attributeEntry);
	const uint8_t paletteOffset = (tileX & 0x2) | ((tileY & 0x2) << 1);
	const uint8_t paletteEntry = (attributeTableValue >> paletteOffset) & 0x3;

	return { value, paletteEntry };
}

NESPPU::PixelOutput NESPPU::generateSprite(uint8_t x, uint8_t y)
{
	auto result = PixelOutput { 0, 0, 0 };
	for (size_t i = 0; i < 8; ++i) {
		if (spriteData[i].x > 0) {
			--spriteData[i].x;
		} else {
			const uint8_t value = (spriteData[i].patternTable0 & 0x1) | ((spriteData[i].patternTable1 & 0x1) << 1);
			const uint8_t palette = spriteData[i].attributes & 0x3;
			const uint8_t priority = (spriteData[i].attributes >> 5) & 0x1;
			spriteData[i].patternTable0 >>= 1;
			spriteData[i].patternTable1 >>= 1;
			if (value != 0 && result.value == 0) {
				result.value = value;
				result.palette = uint8_t(palette + 4);
				result.priority = priority;
			}
		}
	}
	return result;
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

void NESPPU::tickSpriteFetch()
{
	const bool tallSprites = (ppuCtrl & PPUCTRL_SPRITE_SIZE) != 0;

	if (curX == 0) {
		// Do nothing
	} else if (curX <= 64) {
		// Initialize secondary OAM to 0xFF (1-64)
		// This should be timing-correct
		if ((curX & 0x1) == 0) {
			// Write on even cycles
			oamSecondaryData[(curX >> 1) - 1] = 0xFF;
		}
	} else if (curX <= 256) {
		// Sprite evaluation (65-256)
		// Should happen spread between 65 and 256, but doing it all in one go here
		// TIMING ISSUE: if the oamData changes between 65 and 255, the emulation might be incorrect
		if (curX == 256) {
			size_t spriteDst = 0;
			const uint8_t scanline = curY;
			
			for (size_t spriteSrc = 0; spriteSrc < 256; spriteSrc += 4) {
				const uint8_t spriteY = oamData[spriteSrc];
				const uint8_t spriteHeight = 8;
				if (scanline >= uint8_t(spriteY) && scanline < static_cast<uint8_t>(spriteY + spriteHeight)) {
					// In range
					oamSecondaryData[spriteDst] = spriteY;
					oamSecondaryData[spriteDst + 1] = oamData[spriteSrc + 1];
					oamSecondaryData[spriteDst + 2] = oamData[spriteSrc + 2];
					oamSecondaryData[spriteDst + 3] = oamData[spriteSrc + 3];
					spriteDst += 4;

					if (spriteDst == 32) {
						break;
					}
				}
			}
		}
	} else {
		// Sprite fetching (257-320)
		// Should happen between 257-320
		// TIMING ISSUE: if the pattern table changes between 257-319, the emulation might be incorrect
		if (curX == 320) {
			for (size_t i = 0; i < 8; ++i) {
				const uint8_t y = oamSecondaryData[i * 4];
				const uint8_t index = oamSecondaryData[i * 4 + 1] >> (tallSprites ? 1 : 0);
				auto& sprite = spriteData[i];
				sprite.attributes = oamSecondaryData[i * 4 + 2];
				sprite.x = oamSecondaryData[i * 4 + 3];

				const uint8_t pixelYinTile = curY - y;

				const bool flipHorizontal = (sprite.attributes & 0x40) == 0;
				const bool flipVertical = (sprite.attributes & 0x80) == 0;
				
				const uint16_t patternTable = tallSprites ? (index & 0x1) : (ppuCtrl & PPUCTRL_SPRITE_PATTERN_TABLE_ADDRESS) ? 0x1000 : 0x0000;
				sprite.patternTable0 = addressSpace->readDirect(patternTable + (index * 16) + pixelYinTile);
				sprite.patternTable1 = addressSpace->readDirect(patternTable + (index * 16) + pixelYinTile + 8);
				if (flipHorizontal) {
					sprite.patternTable0 = reverseBits(sprite.patternTable0);
					sprite.patternTable1 = reverseBits(sprite.patternTable1);
				}
			}
		}
	}
}

void NESPPU::tickBackgroundFetch()
{
	// TODO
}

bool NESPPU::isRendering() const
{
	return (curY < 240 || curY == 261) && (ppuMask & PPUMASK_SHOW_BACKGROUND || ppuMask & PPUMASK_SHOW_SPRITES);
}

uint8_t NESPPU::reverseBits(uint8_t bits) const
{
	return ((bits & 0x01) << 7)
		| ((bits & 0x02) << 5)
		| ((bits & 0x04) << 3)
		| ((bits & 0x08) << 1)
		| ((bits & 0x10) >> 1)
		| ((bits & 0x20) >> 3)
		| ((bits & 0x40) >> 5)
		| ((bits & 0x80) >> 7);
}
