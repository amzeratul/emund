#include "game_stage.h"

#include "src/nes/nes_rom.h"
#include "src/nes/nes_machine.h"

GameStage::GameStage() = default;

GameStage::~GameStage()
{
	audioStreamHandle->stop();
}

void GameStage::init()
{
	//const char* path = "d:/Emulation/ROMs/NES/nestest.nes";
	//const char* path = "C:/Emulation/ROMs/NES/Donkey Kong (World) (Rev A).nes";
	//const char* path = "d:/Emulation/ROMs/NES/Mega Man 2 (USA).nes";
	const char* path = "C:/Emulation/ROMs/NES/Super Mario Bros. (World).nes";
	//const char* path = "d:/Emulation/ROMs/NES/Balloon Fight (USA).nes";
	
	const auto bytes = Path::readFile(Path(path));
	if (bytes.empty()) {
		throw Exception("Rom not found: " + toString(path), 0);
	}
	
	auto rom = std::make_unique<NESRom>();
	rom->load(gsl::span(reinterpret_cast<const std::byte*>(bytes.data()), bytes.size()));

	nes = std::make_unique<NESMachine>();
	nes->loadROM(std::move(rom));

	setupScreen();
	setupAudio();
	setupInput();
	
	perfView = std::make_shared<PerformanceStatsView>(getResources(), getAPI());
}

void GameStage::onVariableUpdate(Time t)
{
	input->update(t);

	if (getInputAPI().getKeyboard()->isButtonPressed(KeyCode::F2)) {
		perfView->setActive(!perfView->isActive());
	}
	perfView->update();
}

void GameStage::onFixedUpdate(Time t)
{
	std::array<NESInputJoystick, 2> joys;
	fillInput(*input, joys[0]);

	nes->tickFrame(joys);
	generateFrame(nes->getFrameBuffer());
	generateAudio(nes->getAudioBuffer());
}

void GameStage::onRender(RenderContext& rc) const
{
	rc.bind([&] (Painter& painter)
	{
		const auto spriteSize = screen.getSize();
		const auto windowSize = Vector2f(getVideoAPI().getWindow().getDefinition().getSize());
		const auto scales = Vector2i((windowSize / spriteSize).floor());
		const int scale = std::min(scales.x, scales.y);
		
		painter.clear(Colour4f(0.0f, 0.0f, 0.0f));
		if (screen.hasMaterial()) {
			screen
				.clone()
				.setScale(static_cast<float>(scale))
				.setPivot(Vector2f(0.5f, 0.5f))
				.setPosition(windowSize * 0.5f)
				.draw(painter);
		}

		//perfView->paint(painter);
	});
}

void GameStage::generateFrame(gsl::span<const uint32_t> frameBuffer)
{
	texture->startLoading();
	auto texDesc = TextureDescriptor(texture->getSize(), TextureFormat::RGBA);
	texDesc.canBeUpdated = true;
	texDesc.format = TextureFormat::RGBA;
	texDesc.pixelFormat = PixelDataFormat::Image;
	texDesc.pixelData = TextureDescriptorImageData(gsl::as_bytes(frameBuffer));
	texture->load(std::move(texDesc));
}

void GameStage::generateAudio(gsl::span<const float> audioOut)
{
	auto resampled = std::array<float, 1660>();
	auto result = resampler->resample(audioOut, resampled, 0);
	audioStream->addInterleavedSamples(gsl::span<const float>(resampled).subspan(0, result.nWritten));
	//audioStream->addInterleavedSamples(gsl::span<const float>(audioOut).subspan(0, result.nRead));
	
	if (!audioStreamHandle) {
		audioStreamHandle = getAudioAPI().play(audioStream, AudioPosition::makeUI(), 1, true);
	}
}

void GameStage::setupScreen()
{
	const auto textureSize = Vector2i(256, 240);
	texture = getVideoAPI().createTexture(textureSize);
	auto texDesc = TextureDescriptor(textureSize, TextureFormat::RGBA);
	texDesc.canBeUpdated = true;
	texture->load(std::move(texDesc));

	auto material = std::make_shared<Material>(getResources().get<MaterialDefinition>("Halley/SpriteOpaque"));
	material->set(0, texture);

	screen
		.setMaterial(material)
		.setTexRect0(Rect4f(0, 0, 1, 1))
		.setColour(Colour4f(1, 1, 1, 1))
		.setPosition(Vector2f(0, 0))
		.setSize(Vector2f(textureSize));
}

void GameStage::setupAudio()
{
	std::array<float, 1000> buffer;
	buffer.fill(0);
	audioStream = std::make_shared<StreamingAudioClip>(1);
	audioStream->addInterleavedSamples(buffer);
	resampler = std::make_unique<AudioResampler>(99432, 48000, 1, 1.0f);
}

void GameStage::setupInput()
{
	auto kb = getInputAPI().getKeyboard();
	
	input = std::make_shared<InputVirtual>(4, 2);
	input->bindButton(0, kb, Keys::A); // A
	input->bindButton(1, kb, Keys::S); // B
	input->bindButton(2, kb, Keys::Space); // Select
	input->bindButton(3, kb, Keys::Enter); // Enter
	input->bindAxisButton(0, kb, Keys::Left, Keys::Right);
	input->bindAxisButton(1, kb, Keys::Down, Keys::Up);
}

void GameStage::fillInput(InputDevice& src, NESInputJoystick& dst)
{
	dst.left = (src.getAxis(0) < -0.5f) ? 1 : 0;
	dst.right = (src.getAxis(0) > 0.5f) ? 1 : 0;
	dst.down = (src.getAxis(1) < -0.5f) ? 1 : 0;
	dst.up = (src.getAxis(1) > 0.5f) ? 1 : 0;
	dst.a = src.isButtonDown(0);
	dst.b = src.isButtonDown(1);
	dst.select = src.isButtonDown(2);
	dst.start = src.isButtonDown(3);
}
