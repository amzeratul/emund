#include "nes_ppu.h"

#include "src/cpu/address_space.h"

NESPPU::NESPPU()
{
	ppuStatus.value = 0;
	ppuStatus.vBlank = true;
}

bool NESPPU::tick()
{
	generatePixel();
	
	++cycle;
	++curX;

	// Last scanline on odd fields is shorter
	// http://wiki.nesdev.com/w/index.php/PPU_frame_timing
	const uint32_t scanLen = curY == 262 && frameN % 2 == 1 ? 340 : 341;
	
	if (curX == scanLen) {
		curX = 0;
		++curY;

		if (curY == 20) {
			ppuStatus.vBlank = false;
		}
		
		if (curY == 262) {
			frameN++;
			curY = 0;
			ppuStatus.vBlank = true;
			return true;
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

uint32_t NESPPU::getCycle() const
{
	return cycle;
}

void NESPPU::mapRegisters(AddressSpace8BitBy16Bit& addressSpace)
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
			const uint8_t value = ppuStatus.value;
			ppuStatus.vBlank = false;
			return value;
		}
	}
	return 0;
}

void NESPPU::writeRegister(uint16_t address, uint8_t value)
{
	// TODO
}

void NESPPU::setFrameBuffer(gsl::span<uint8_t> fb)
{
	frameBuffer = fb;
}

void NESPPU::generatePixel()
{
	// TODO
	frameBuffer[curX + curY * 341] = frameN & 0xFF;
}
