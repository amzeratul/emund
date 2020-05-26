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

		if (curY == 240) {
			ppuStatus.vBlank = true;
		}
		
		if (curY == 262) {
			frameN++;
			curY = 0;
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
			const uint8_t value = ppuStatus.value;
			ppuStatus.vBlank = false;
			ppuScrollIdx = 0; // Should I do this here?
			return value;
		}
	case 0x2007:
		{
			const uint8_t value = addressSpace->read(ppuAddr);
			ppuAddr += ppuCtrl.vramAddressIncrement == PPUCTRL::AddressIncrement::Horizontal ? 1 : 32;
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
		ppuCtrl.value = value;
		break;
	case 0x2001:
		ppuMask.value = value;
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
		ppuAddr += ppuCtrl.vramAddressIncrement == PPUCTRL::AddressIncrement::Horizontal ? 1 : 32;
		break;
	default:
		throw std::exception();
	}
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
