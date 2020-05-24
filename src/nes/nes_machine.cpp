#include "nes_machine.h"

#include "src/cpu/cpu_6502.h"
#include "src/cpu/address_space.h"
#include "src/nes/nes_mapper.h"
#include "src/nes/nes_rom.h"

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
			auto error = cpu->getError();
			auto instruction = cpu->getErrorInstruction();
			throw std::exception("CPU error");
		}
	}
}
