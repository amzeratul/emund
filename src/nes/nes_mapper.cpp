#include "nes_mapper.h"
#include "nes_rom.h"
#include "src/cpu/address_space.h"

bool NESMapper::map(NESRom& rom, AddressSpace8BitBy16Bit& cpuAddressSpace, AddressSpace8BitBy16Bit& ppuAddressSpace)
{
	switch (rom.getMapper()) {
	case 0:
		mapper0(rom, cpuAddressSpace, ppuAddressSpace);
		return true;

	default:
		return false;
	}
}

void NESMapper::mapper0(NESRom& rom, AddressSpace8BitBy16Bit& cpuAddressSpace, AddressSpace8BitBy16Bit& ppuAddressSpace)
{
	cpuAddressSpace.map(rom.getPRGROM(), 0x8000, 0xFFFF);
	ppuAddressSpace.map(rom.getCHRROM(), 0x0000, 0x1FFF);	
}
