#pragma once
#include <memory>
#include <vector>

class NESRom;
class CPU6502;
class AddressSpace8BitBy16Bit;

class NESMachine {
public:
	NESMachine();
	~NESMachine();

	void loadROM(std::unique_ptr<NESRom> rom);
	void tick(double t);

private:
	std::unique_ptr<NESRom> rom;
	std::unique_ptr<CPU6502> cpu;
	std::unique_ptr<AddressSpace8BitBy16Bit> addressSpace;
	std::vector<uint8_t> ram;
};

