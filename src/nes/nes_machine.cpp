#include "nes_machine.h"
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
}

NESMachine::~NESMachine() = default;

void NESMachine::loadROM(std::unique_ptr<NESRom> romToLoad)
{
	rom = std::move(romToLoad);
	NESMapper mapper;
	mapper.map(*rom, *addressSpace);
}

void NESMachine::tick(double t)
{
	while (!cpu->hasError()) {
		cpu->tick();

		if (cpu->hasError()) {			
			switch (cpu->getError()) {
			case CPU6502::ErrorType::Break:
				Logger::logError("Break instruction reached");
				break;
			case CPU6502::ErrorType::UnknownInstruction:
				Logger::logError("Unknown instruction: " + toString(uint32_t(cpu->getErrorInstruction()), 16));
			}

			// Memory dump
			constexpr size_t bytesPerRow = 16;
			Logger::logDev("\nRAM dump:");
			Logger::logDev("     00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F");
			for (size_t i = 0; i < 2048; i += bytesPerRow) {
				String row = toString(i, 16, 3) + ": ";
				for (size_t j = 0; j < bytesPerRow; ++j) {
					row += toString(uint32_t(ram[i + j]), 16, 2).asciiUpper() + " ";
				}
				Logger::logDev(row);
			}
		}
	}
}
