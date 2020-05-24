#pragma once

class AddressSpace8BitBy16Bit;

class CPU6502 {
public:
	void setAddressSpace(AddressSpace8BitBy16Bit& addressSpace);
	void tick();

private:
	AddressSpace8BitBy16Bit* addressSpace = nullptr;
};
