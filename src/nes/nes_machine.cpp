#include "nes_machine.h"
#include "nes_ppu.h"
#include "src/cpu/cpu_6502.h"
#include "src/cpu/address_space.h"
#include "src/nes/nes_mapper.h"
#include "src/nes/nes_rom.h"

#include <halley.hpp>

#include "nes_apu.h"

using namespace Halley;

NESInputJoystick::NESInputJoystick()
{
	clear();
}

uint8_t NESInputJoystick::toBits() const
{
	return (a << 0)
		| (b << 1)
		| (select << 2)
		| (start << 3)
		| (up << 4)
		| (down << 5)
		| (left << 6)
		| (right << 7);
}

void NESInputJoystick::clear()
{
	a = 0;
	b = 0;
	select = 0;
	start = 0;
	up = 0;
	down = 0;
	left = 0;
	right = 0;
}

NESMachine::NESMachine()
{
	frameBuffer.resize(256 * 240);
	
	cpuAddressSpace = std::make_unique<AddressSpace8BitBy16Bit>();
	ram.resize(2 * 1024, 0);
	cpuAddressSpace->map(ram, 0x0000, 0x1FFF);

	ppuAddressSpace = std::make_unique<AddressSpace8BitBy16Bit>();
	vram.resize(2 * 1024, 0);
	paletteRam.resize(32, 0);
	ppuAddressSpace->map(gsl::span<uint8_t>(vram), 0x2000, 0x3EFF);
	ppuAddressSpace->map(paletteRam, 0x3F00, 0x3FFF, 0x1F);

	cpu = std::make_unique<CPU6502>();
	cpu->setAddressSpace(*cpuAddressSpace);

	ppu = std::make_unique<NESPPU>();
	ppu->mapRegistersOnCPUAddressSpace(*cpuAddressSpace);
	ppu->setAddressSpace(*ppuAddressSpace);
	ppu->setFrameBuffer(frameBuffer);

	apu = std::make_unique<NESAPU>();

	cpuAddressSpace->mapRegister(0x4000, 0x401F, this, [] (void* self, uint16_t address, uint8_t& value, bool write)
	{
		const auto machine = static_cast<NESMachine*>(self);
		if (write) {
			machine->writeRegister(address, value);
		} else {
			value = machine->readRegister(address);
		}
	});
}

NESMachine::~NESMachine() = default;

void NESMachine::loadROM(std::unique_ptr<NESRom> romToLoad)
{
	rom = std::move(romToLoad);
	mapper = std::make_unique<NESMapper>();
	if (!mapper->map(*rom, *cpuAddressSpace, *ppuAddressSpace)) {
		Logger::logError("Unknown mapper: " + toString(rom->getMapper()));
	}
	cpu->raiseReset();
	running = true;
}

void NESMachine::tickFrame(NESInputJoystick joy0, NESInputJoystick joy1)
{
	while (running) {
		// Step APU first, have it catch up to CPU
		const auto targetAPUCycle = cpu->getCycle() / 2;
		while (apu->getCycle() < targetAPUCycle) {
			apu->tick();
		}
		
		// Step PPU next, have it also catch up to CPU
		const auto targetPPUCycle = cpu->getCycle() * 3;
		while (ppu->getCycle() < targetPPUCycle) {
			const bool vsync = ppu->tick();
			if (vsync) {
				// If we finish a frame, stop here and render it out before continuing
				if (ppu->canGenerateNMI()) {
					cpu->raiseNMI();
				}

				//Logger::logInfo("Frame " + toString(ppu->getFrameNumber()) + ": " + toString(cpu->getCycle() - startCPU) + ", total: " + toString(cpu->getCycle()) + ", average: " + toString(cpu->getCycle() / (ppu->getFrameNumber() + 1)));
				return;
			}
		}

		// Tick CPU
		//cpu->printDebugInfo();
		cpu->tick();
		if (cpu->hasError()) {
			running = false;
			reportCPUError();
			return;
		}

		// Latch input
		if (inputLatch & 1) {
			port0 = joy0.toBits();
			port1 = joy1.toBits();
		}
	}
}

uint8_t NESMachine::readRegister(uint16_t address)
{
	switch (address) {
	case 0x4015:
		return apu->readRegister(address);
	case 0x4016:
		{
			const uint8_t value = (port0 & 1);
			port0 = (port0 >> 1) | 0x80;
			return value;
		}
	case 0x4017:
		{
			const uint8_t value = (port1 & 1);
			port1 = (port1 >> 1) | 0x80;
			return value;
		}
	default:
		return 0;
	}
}

void NESMachine::writeRegister(uint16_t address, uint8_t value)
{
	switch (address) {
	case 0x4014:
		// OAMDMA
		cpu->copyOAM(value, ppu->getOAMData());
		break;
	case 0x4016:
		// JOY1
	    inputLatch = value & 0x7;
	    break;
	default:
		apu->writeRegister(address, value);
		break;
	}
}

gsl::span<const uint32_t> NESMachine::getFrameBuffer() const
{
	return frameBuffer;
}

void NESMachine::reportCPUError()
{
	switch (cpu->getError()) {
	case CPU6502::ErrorType::OK:
		break;
	case CPU6502::ErrorType::Break:
		Logger::logError("Break instruction reached");
		break;
	case CPU6502::ErrorType::UnknownInstruction:
		Logger::logError("Unknown instruction: $" + toString(static_cast<int>(cpu->getErrorInstruction()), 16, 2).asciiUpper());
	}

	cpuAddressSpace->dump(0x0000, 0x1FFF);
}
