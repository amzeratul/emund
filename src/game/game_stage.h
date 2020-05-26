#pragma once

#include <halley.hpp>

using namespace Halley;

class NESMachine;

class GameStage : public EntityStage {
public:
	GameStage();
	~GameStage();
	
	void init() override;

	void onFixedUpdate(Time) override;
	void onRender(RenderContext&) const override;

private:
	std::unique_ptr<NESMachine> nes;

	Sprite screen;
	std::shared_ptr<Texture> texture;

	void generateFrame(gsl::span<const uint8_t> frameBuffer);
	void setupScreen();
};
