#pragma once

#include <halley.hpp>

using namespace Halley;

class NESMachine;

class GameStage : public EntityStage {
public:
	GameStage();
	~GameStage();
	
	void init() override;

	void onVariableUpdate(Time) override;
	void onRender(RenderContext&) const override;

private:
	std::unique_ptr<NESMachine> nes;
};
