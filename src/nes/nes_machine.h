#pragma once
#include <memory>
#include <vector>
#include <gsl/span>


class NESRom;
class NESMapper;
class CPU6502;
class NESPPU;
class AddressSpace8BitBy16Bit;

struct NESInputJoystick {
	uint8_t a : 1;
	uint8_t b : 1;
	uint8_t select : 1;
	uint8_t start : 1;
	uint8_t up : 1;
	uint8_t down : 1;
	uint8_t left : 1;
	uint8_t right : 1;

	NESInputJoystick();
	uint8_t toBits() const;
	void clear();
};

class NESMachine {
public:
	NESMachine();
	~NESMachine();

	void loadROM(std::unique_ptr<NESRom> rom);
	void tickFrame(NESInputJoystick joy0, NESInputJoystick joy1);

	uint8_t readRegister(uint16_t address);
	void writeRegister(uint16_t address, uint8_t value);

	gsl::span<const uint32_t> getFrameBuffer() const;

private:
	bool running = false;
	
	std::unique_ptr<NESRom> rom;
	std::unique_ptr<NESMapper> mapper;
	std::unique_ptr<CPU6502> cpu;
	std::unique_ptr<NESPPU> ppu;
	std::unique_ptr<AddressSpace8BitBy16Bit> cpuAddressSpace;
	std::unique_ptr<AddressSpace8BitBy16Bit> ppuAddressSpace;
	std::vector<uint8_t> ram;
	std::vector<uint8_t> vram;
	std::vector<uint8_t> paletteRam;

	std::vector<uint32_t> frameBuffer;

	uint8_t inputLatch = 0;
	uint8_t port0 = 0;
	uint8_t port1 = 0;

	void reportCPUError();
};

