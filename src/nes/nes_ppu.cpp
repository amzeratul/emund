#include "nes_ppu.h"

#include "src/cpu/address_space.h"

NESPPU::NESPPU()
{
	ppuStatus.value = 0;
	ppuStatus.vBlank = true;
}

void NESPPU::tick()
{
	++cycle;
}

uint32_t NESPPU::getCycle()
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

void NESPPU::startVBlank()
{
	ppuStatus.vBlank = true;
}
