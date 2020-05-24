#pragma once

#include <halley.hpp>

using namespace Halley;

class NESRom;
class CPU6502;

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
};
