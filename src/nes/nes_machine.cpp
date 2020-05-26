#include "nes_machine.h"
#include "nes_ppu.h"
#include "src/cpu/cpu_6502.h"
#include "src/cpu/address_space.h"
#include "src/nes/nes_mapper.h"
#include "src/nes/nes_rom.h"

#include <halley.hpp>

using namespace Halley;

NESMachine::NESMachine()
{
	frameBuffer.resize(361 * 261);
	
	addressSpace = std::make_unique<AddressSpace8BitBy16Bit>();

	ram.resize(2 * 1024, 0);
	addressSpace->map(ram, 0x0000, 0x1FFF);

	cpu = std::make_unique<CPU6502>();
	cpu->setAddressSpace(*addressSpace);

	ppu = std::make_unique<NESPPU>();
	ppu->mapRegisters(*addressSpace);
	ppu->setFrameBuffer(frameBuffer);

	addressSpace->mapRegister(0x4000, 0x401F, this, [] (void* self, uint16_t address, uint8_t& value, bool write)
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
	if (!mapper->map(*rom, *addressSpace)) {
		Logger::logError("Unknown mapper: " + toString(rom->getMapper()));
	}
	cpu->reset();
	running = true;
}

void NESMachine::tickFrame()
{
	/*
	const uint32_t masterClockCycles = 357366;
	const uint32_t nCPUCycles = masterClockCycles / 12;
	const uint32_t startCPU = cpu->getCycle();
	const uint32_t endCPU = startCPU + nCPUCycles;
	*/
	
	while (running) {
		// Step PPU first, have it catch up to CPU
		const auto targetPPUCycle = cpu->getCycle() * 3;
		while (ppu->getCycle() < targetPPUCycle) {
			const bool vsync = ppu->tick();
			if (vsync) {
				// If we finish a frame, stop here and render it out before continuing
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
	}
}

uint8_t NESMachine::readRegister(uint16_t address)
{
	// TODO
	return 0;
}

void NESMachine::writeRegister(uint16_t address, uint8_t value)
{
	// TODO
}

gsl::span<const uint8_t> NESMachine::getFrameBuffer() const
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

	addressSpace->dump(0x0000, 0x1FFF);
}
