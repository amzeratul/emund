#pragma once

#include <halley.hpp>

using namespace Halley;

class NESRom;
class CPU6502;
class AddressSpace8BitBy16Bit;

class GameStage : public EntityStage {
public:
	GameStage();
	~GameStage();
	
	void init() override;

	void onVariableUpdate(Time) override;
	void onRender(RenderContext&) const override;

private:
	std::unique_ptr<NESRom> rom;
	std::unique_ptr<CPU6502> cpu;
	std::unique_ptr<AddressSpace8BitBy16Bit> addressSpace;
};
