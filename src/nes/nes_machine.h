#pragma once
#include <memory>
#include <vector>

class NESRom;
class NESMapper;
class CPU6502;
class NESPPU;
class AddressSpace8BitBy16Bit;

class NESMachine {
public:
	NESMachine();
	~NESMachine();

	void loadROM(std::unique_ptr<NESRom> rom);
	void tickFrame();

	uint8_t readRegister(uint16_t address);
	void writeRegister(uint16_t address, uint8_t value);

private:
	bool running = false;
	
	std::unique_ptr<NESRom> rom;
	std::unique_ptr<NESMapper> mapper;
	std::unique_ptr<CPU6502> cpu;
	std::unique_ptr<NESPPU> ppu;
	std::unique_ptr<AddressSpace8BitBy16Bit> addressSpace;
	std::vector<uint8_t> ram;
};

