#include "nes_ppu.h"
#include "src/cpu/address_space.h"

#include <halley.hpp>

#include "src/utils/bit_view.h"
using namespace Halley;

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


using RegisterCoarseX = BitView<uint16_t, 0, 5>;
using RegisterCoarseY = BitView<uint16_t, 5, 5>;
using RegisterNametableSelect = BitView<uint16_t, 10, 2>;
using RegisterNametableSelectX = BitView<uint16_t, 10, 1>;
using RegisterNametableSelectY = BitView<uint16_t, 11, 1>;
using RegisterFineY = BitView<uint16_t, 12, 3>;
using RegisterFineX = BitView<uint8_t, 0, 3>;
using RegisterTileAddress = BitView<uint16_t, 0, 12>;

using RegisterAddress = BitView<uint16_t, 0, 14>;
using RegisterAddressHigh = BitView<uint16_t, 8, 6>;
using RegisterAddressLow = BitView<uint16_t, 0, 8>;

using CtrlNametableSelect = BitView<uint8_t, 0, 2>;

using ParFineScroll = BitView<uint8_t, 0, 3>;
using ParCoarseScroll = BitView<uint8_t, 3, 5>;

using ShiftRegisterTop = BitView<uint16_t, 8, 8>;
using ShiftRegisterBottom = BitView<uint16_t, 0, 1>;


NESPPU::NESPPU()
{
	ppuStatus = 0;
	oamData.resize(256, 0);
	oamSecondaryData.resize(32, 0);
}

bool NESPPU::tick()
{
	const bool isPreRenderLine = curY == 261;
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
	
	if (isPreRenderLine && curX == 1) {
		ppuStatus &= ~(PPUSTATUS_SPRITE_ZERO_HIT | PPUSTATUS_VBLANK | PPUSTATUS_SPRITE_OVERFLOW);
	}

	if (isVisibleLine) {
		if (curX == 257) {
			// Update horizontal scroll
			RegisterCoarseX::of(vRegister).set(RegisterCoarseX::of(tRegister));
			RegisterNametableSelectX::of(vRegister).set(RegisterNametableSelectX::of(tRegister));
		}
	}

	if (isPreRenderLine && curX >= 280 && curX < 304) {
		// Update vertical scroll
		RegisterCoarseY::of(vRegister).set(RegisterCoarseY::of(tRegister));
		RegisterFineY::of(vRegister).set(RegisterFineY::of(tRegister));
		RegisterNametableSelectY::of(vRegister).set(RegisterNametableSelectY::of(tRegister));
	}

	// Last scanline on odd fields is shorter
	// http://wiki.nesdev.com/w/index.php/PPU_frame_timing
	const uint32_t scanLen = isPreRenderLine && frameN % 2 == 1 ? 340 : 341;

	// Step
	++cycle;
	++curX;	

	// VBlank happens on the line after post-render
	if (curY == 241 && curX == 1) {
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
		}
	}
	
	return false;
}

void NESPPU::incrementHorizontalPos()
{
	auto coarseX = RegisterCoarseX(vRegister);
	coarseX += 1;
	if (coarseX.getValue() == 0) {
		auto nametable = RegisterNametableSelectX(vRegister);
		nametable.set(uint16_t(1 - nametable.getValue()));
	}
}

void NESPPU::incrementVerticalPos()
{
	auto fineY = RegisterFineY(vRegister);
	fineY += 1;
	if (fineY.getValue() == 0) {
		auto coarseY = RegisterCoarseY(vRegister);
		coarseY += 1;
		if (coarseY.getValue() == 30) {
			coarseY.set(uint8_t(0));
			auto nametable = RegisterNametableSelectY(vRegister);
			nametable.set(uint16_t(1 - nametable.getValue()));
		}
	}
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
			wRegister = false;
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
			const uint8_t value = ppuDataBuffer;
			RegisterAddress ppuAddr(vRegister);
			ppuDataBuffer = readByte(ppuAddr.getValue());
			ppuAddr += static_cast<uint16_t>((ppuCtrl & PPUCTRL_VRAM_ADDRESS) ? 32 : 1);
			if (ppuAddr.getValue() >= 0x3f00) {
				// Palettes are placed on data bus immediately, for some reason
				return ppuDataBuffer;
			}
			return value;
		}
	default:
		throw std::exception();
	}
	return 0;
}

void NESPPU::writeRegister(uint16_t address, uint8_t value)
{
	bool isReady = cycle >= 88974;
	
	switch (address) {
	case 0x2000:
		if (isReady) {
			ppuCtrl = value;
			RegisterNametableSelect(tRegister).set(CtrlNametableSelect(value));
		}
		break;
	case 0x2001:
		if (isReady) {
			ppuMask = value;
		}
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
		if (isReady) {
			if (!wRegister) {
				RegisterCoarseX(tRegister).set(ParCoarseScroll(value));
				RegisterFineX(xRegister).set(ParFineScroll(value));
			} else {
				RegisterCoarseY(tRegister).set(ParCoarseScroll(value));
				RegisterFineY(tRegister).set(ParFineScroll(value));
			}
			wRegister = !wRegister;
		}
		break;
	case 0x2006:
		if (isReady) {
			if (!wRegister) {
				RegisterAddressHigh(tRegister).set(value);
			} else {
				RegisterAddressLow(tRegister).set(value);
				vRegister = tRegister;
			}
			wRegister = !wRegister;
		}
		break;
	case 0x2007:
		{
			auto addr = RegisterAddress(vRegister);
			writeByte(addr.getValue(), value);
			addr += (ppuCtrl & PPUCTRL_VRAM_ADDRESS) ? 32 : 1;
		}
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

uint32_t NESPPU::getFrameNumber() const
{
	return frameN;
}

void NESPPU::generatePixel(uint8_t x, uint8_t y)
{
	auto bg = generateBackground(x, y);
	auto sprite = generateSprite(x, y);

	PixelOutput result;
	if (sprite.value != 0 && bg.value != 0) {
		result = sprite.priority == 0 ? sprite : bg;
		if (sprite.spriteN == 0) {
			ppuStatus |= PPUSTATUS_SPRITE_ZERO_HIT;
		}
	} else if (sprite.value != 0) {
		result = sprite;
	} else if (bg.value != 0) {
		result = bg;
	} else {
		result = { bg.value, 0, 0, 0 };
	}
	
	const uint8_t colour = addressSpace->readDirect(0x3F00 + 4 * result.palette + result.value);

	frameBuffer[size_t(x) + size_t(y) * 256] = paletteToColour(colour);
}

NESPPU::PixelOutput NESPPU::generateBackground(uint8_t x, uint8_t y)
{
	if (!(ppuMask & PPUMASK_SHOW_BACKGROUND) || (x < 8 && !(ppuMask & PPUMASK_SHOW_BACKGROUND_LEFT))) {
		return PixelOutput { 0, 0, 0, 0 };
	}

	// Fetch data from registers
	auto select = [&](auto v) -> uint8_t
	{
		return static_cast<uint8_t>((v >> xRegister) & 1);
	};	
	const uint8_t value = (select(patternTableHighShiftRegister) << 1) | select(patternTableLowShiftRegister);
	const uint8_t paletteEntry = (select(attributeHighShiftRegister) << 1) | select(attributeLowShiftRegister);

	// Shift registers
	auto shiftAndLoad = [](uint8_t& shiftRegister, uint8_t data)
	{
		shiftRegister = (shiftRegister >> 1) | (data << 7);
	};
	shiftAndLoad(attributeHighShiftRegister, (attributeLatch & 0x2) >> 1);
	shiftAndLoad(attributeLowShiftRegister, attributeLatch & 0x1);
	patternTableHighShiftRegister >>= 1;
	patternTableLowShiftRegister >>= 1;

	return { value, paletteEntry, 0, 0 };
}

NESPPU::PixelOutput NESPPU::generateSprite(uint8_t x, uint8_t y)
{
	if (!(ppuMask & PPUMASK_SHOW_SPRITES) || (x < 8 && !(ppuMask & PPUMASK_SHOW_SPRITES_LEFT))) {
		return PixelOutput { 0, 0, 0, 0 };
	}
	
	auto result = PixelOutput { 0, 0, 0, 0 };
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
				result.spriteN = uint8_t(i);
			}
		}
	}
	return result;
}

uint32_t NESPPU::paletteToColour(uint8_t palette)
{
	const static uint32_t entries[] = {
		0x7C7C7C,
		0xFC0000,
		0xBC0000,
		0xBC2844,
		0x840094,
		0x2000A8,
		0x0010A8,
		0x001488,
		0x003050,
		0x007800,
		0x006800,
		0x005800,
		0x584000,
		0x000000,
		0x000000,
		0x000000,
		0xBCBCBC,
		0xF87800,
		0xF85800,
		0xFC4468,
		0xCC00D8,
		0x5800E4,
		0x0038F8,
		0x105CE4,
		0x007CAC,
		0x00B800,
		0x00A800,
		0x44A800,
		0x888800,
		0x000000,
		0x000000,
		0x000000,
		0xF8F8F8,
		0xFCBC3C,
		0xFC8868,
		0xF87898,
		0xF878F8,
		0x9858F8,
		0x5878F8,
		0x44A0FC,
		0x00B8F8,
		0x18F8B8,
		0x54D858,
		0x98F858,
		0xD8E800,
		0x787878,
		0x000000,
		0x000000,
		0xFCFCFC,
		0xFCE4A4,
		0xF8B8B8,
		0xF8B8D8,
		0xF8B8F8,
		0xC0A4F8,
		0xB0D0F0,
		0xA8E0FC,
		0x78D8F8,
		0x78F8D8,
		0xB8F8B8,
		0xD8F8B8,
		0xFCFC00,
		0xD8D8D8,
		0x000000,
		0x000000,
	};

	return entries[palette];
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

				const bool flipHorizontal = (sprite.attributes & 0x40) == 0;
				const bool flipVertical = (sprite.attributes & 0x80) != 0;

				const uint8_t pixelYinTile = flipVertical ? (7 - curY + y) : curY - y;
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
	// Background fetching
	if ((curX >= 1 && curX < 257) || curX >= 321) {
		if (curX % 8 == 0) {
			incrementHorizontalPos();
			if (curX == 256) {
				incrementVerticalPos();
			}
		} else if (curX % 8 == 1) {
			nameTableLatch = readByte(RegisterTileAddress(vRegister).getValue() | 0x2000);
		} else if (curX % 8 == 3) {
			const uint16_t addr = 0x23C0 | (vRegister & 0x0C00) | ((vRegister >> 4) & 0x38) | ((vRegister >> 2) & 0x07);
			const uint8_t value = readByte(addr);
			// const uint8_t paletteOffset = (pageX & 0x2) | ((pageY & 0x2) << 1);
			// const uint8_t paletteEntry = (attributeTableValue >> paletteOffset) & 0x3;
			const uint8_t paletteOffset = (RegisterCoarseX(vRegister).getValue() & 0x2) | ((RegisterCoarseY(vRegister).getValue() & 0x2) << 1);
			attributeLatch = (value >> paletteOffset) & 0x3;
		} else {
			const uint16_t patternTable = (ppuCtrl & PPUCTRL_BACKGROUND_PATTERN_TABLE_ADDRESS) ? 0x1000 : 0x0000;
			const uint16_t addr = patternTable | nameTableLatch * 16 | RegisterFineY(vRegister).getValue();
			if (curX % 8 == 5) {
				patternTableLowLatch = readByte(addr);
			} else if (curX % 8 == 7) {
				patternTableHighLatch = readByte(addr + 8);
			}
		}
	}

	// Shift registers
	if ((curX >= 2 && curX < 257) || curX >= 322) {
		if (curX % 8 == 1) {
			// Load shift registers
			attributeHighShiftRegister = (attributeLatch % 4 == 2) ? 0xFF : 0x00;
			attributeLowShiftRegister = (attributeLatch % 2 == 1) ? 0xFF : 0x00;
			ShiftRegisterTop(patternTableHighShiftRegister).set(patternTableHighLatch);
			ShiftRegisterTop(patternTableLowShiftRegister).set(patternTableLowLatch);
		}
	}
}

void NESPPU::writeByte(uint16_t address, uint8_t value)
{
	// This is to address the insane palette mirroring
	// 0x3F10 -> 0x3F00
	// 0x3F14 -> 0x3F04
	// 0x3F18 -> 0x3F08
	// 0x3F1C -> 0x3F0C
	// ONLY those four addresses map in this way
	if (address >= 0x3F10 && address <= 0x3F1C && (address & 0x3) == 0) {
		address -= 0x10;
	}
	addressSpace->write(address, value);
}

uint8_t NESPPU::readByte(uint16_t address)
{
	// See note above
	if (address >= 0x3F10 && address <= 0x3F1C && (address & 0x3) == 0) {
		address -= 0x10;
	}
	return addressSpace->read(address);
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
