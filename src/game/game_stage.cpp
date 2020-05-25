#include "game_stage.h"

#include "src/nes/nes_rom.h"
#include "src/nes/nes_machine.h"

GameStage::GameStage() = default;

GameStage::~GameStage() = default;

void GameStage::init()
{
	//const char* path = "d:/Emulation/ROMs/NES/nestest.nes";
	const char* path = "d:/Emulation/ROMs/NES/Donkey Kong (World) (Rev 1).nes";
	
	const auto bytes = Path::readFile(Path(path));
	auto rom = std::make_unique<NESRom>();
	rom->load(gsl::span(reinterpret_cast<const std::byte*>(bytes.data()), bytes.size()));

	nes = std::make_unique<NESMachine>();
	nes->loadROM(std::move(rom));
}

void GameStage::onVariableUpdate(Time t)
{
	nes->tick(t);
}

void GameStage::onRender(RenderContext& rc) const
{
}
