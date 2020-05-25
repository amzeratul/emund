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
	addressSpace = std::make_unique<AddressSpace8BitBy16Bit>();

	ram.resize(2 * 1024, 0);
	addressSpace->map(ram, 0x0000, 0x1FFF);

	cpu = std::make_unique<CPU6502>();
	cpu->setAddressSpace(*addressSpace);

	ppu = std::make_unique<NESPPU>();
	ppu->mapRegisters(*addressSpace);
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

void NESMachine::tick(double t)
{
	if (!running) {
		return;
	}
	
	while (running) {
		cpu->printDebugInfo();
		cpu->tick();

		if (cpu->hasError()) {			
			running = false;

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
			return;
		}

		const auto targetPPUCycle = cpu->getCycle() * 3;
		while (ppu->getCycle() < targetPPUCycle) {
			ppu->tick();
		}
	}
}
