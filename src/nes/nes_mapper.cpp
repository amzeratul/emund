#include "nes_mapper.h"
#include "nes_rom.h"
#include "src/cpu/address_space.h"

bool NESMapper::map(NESRom& rom, AddressSpace8BitBy16Bit& addressSpace)
{
	switch (rom.getMapper()) {
	case 0:
		mapper0(rom, addressSpace);
		return true;

	default:
		return false;
	}
}

void NESMapper::mapper0(NESRom& rom, AddressSpace8BitBy16Bit& addressSpace)
{
	const auto prgRom = rom.getPRGROM();
	addressSpace.map(prgRom, 0x8000, 0xFFFF);
}
