#pragma once

class AddressSpace8BitBy16Bit;
class NESRom;

class NESMapper {
public:
	bool map(NESRom& rom, AddressSpace8BitBy16Bit& addressSpace);

private:
	void mapper0(NESRom& rom, AddressSpace8BitBy16Bit& addressSpace);
};
