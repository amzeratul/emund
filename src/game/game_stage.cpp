#include "game_stage.h"

#include "src/nes/nes_rom.h"
#include "src/cpu/cpu_6502.h"
#include "src/cpu/address_space.h"

GameStage::GameStage() = default;

GameStage::~GameStage() = default;

void GameStage::init()
{
	const auto bytes = Path::readFile(Path("d:/Emulation/ROMs/NES/nestest.nes"));
	rom = std::make_unique<NESRom>();
	rom->load(gsl::span(reinterpret_cast<const std::byte*>(bytes.data()), bytes.size()));

	addressSpace = std::make_unique<AddressSpace8BitBy16Bit>();
	addressSpace->map(rom->getPRGROM(), 0xC000);

	cpu = std::make_unique<CPU6502>();
	cpu->setAddressSpace(*addressSpace);
	
	while (true) {
		cpu->tick();
	}
}

void GameStage::onVariableUpdate(Time t)
{
}

void GameStage::onRender(RenderContext& rc) const
{
}
