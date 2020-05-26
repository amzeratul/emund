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

	setupScreen();
}

void GameStage::onFixedUpdate(Time)
{
	nes->tickFrame();
	generateFrame(nes->getFrameBuffer());
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
	});
}

void GameStage::generateFrame(gsl::span<const uint8_t> frameBuffer)
{
	texture->startLoading();
	auto texDesc = TextureDescriptor(texture->getSize(), TextureFormat::Red);
	texDesc.canBeUpdated = true;
	texDesc.format = TextureFormat::Red;
	texDesc.pixelFormat = PixelDataFormat::Image;
	texDesc.pixelData = TextureDescriptorImageData(gsl::as_bytes(frameBuffer));
	texture->load(std::move(texDesc));
}

void GameStage::setupScreen()
{
	const auto textureSize = Vector2i(341, 261);
	texture = getVideoAPI().createTexture(textureSize);
	auto texDesc = TextureDescriptor(textureSize, TextureFormat::Red);
	texDesc.canBeUpdated = true;
	texture->load(std::move(texDesc));

	auto material = std::make_shared<Material>(getResources().get<MaterialDefinition>("Halley/SpriteOpaque"));
	material->set("tex0", texture);

	screen
		.setMaterial(material)
		.setPosition(Vector2f(0, 0))
		.setSize(Vector2f(textureSize));
}
