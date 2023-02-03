#pragma once

#include <halley.hpp>

#include "halley/audio/resampler.h"

struct NESInputJoystick;
using namespace Halley;

class NESMachine;

class GameStage : public EntityStage {
public:
	GameStage();
	~GameStage();
	
	void init() override;

	void onVariableUpdate(Time) override;
	void onFixedUpdate(Time) override;
	void onRender(RenderContext&) const override;

private:
	std::unique_ptr<NESMachine> nes;

	Sprite screen;
	std::shared_ptr<Texture> texture;
	std::shared_ptr<InputVirtual> input;

	std::shared_ptr<PerformanceStatsView> perfView;
	std::shared_ptr<StreamingAudioClip> audioStream;
	AudioHandle audioStreamHandle;
	std::unique_ptr<AudioResampler> resampler;

	void generateFrame(gsl::span<const uint32_t> frameBuffer);
	void generateAudio(gsl::span<const float> audioOut);

	void setupScreen();
	void setupAudio();
	void setupInput();
	void fillInput(InputDevice& src, NESInputJoystick& dst);
};
