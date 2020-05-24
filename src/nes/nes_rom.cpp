#include "nes_rom.h"

bool NESRom::load(gsl::span<const std::byte> romData)
{
	gsl::span<const std::uint8_t> dataLeft(reinterpret_cast<const uint8_t*>(romData.data()), romData.size());
	
	if (dataLeft.size() < 16) {
		// Not enough for iNES ROM header
		return false;
	}

	// Signature
	const uint8_t signature[4] = { 'N', 'E', 'S', 0x1A };
	if (memcmp(dataLeft.data(), signature, 4) != 0) {
		// Wrong format
		return false;
	}

	// Rom and character rom sizes. They're stored as multiples of 16k and 8k, respectively
	const size_t prgRomSize = static_cast<size_t>(dataLeft[4]) * 16 * 1024;
	const size_t chrRomSize = static_cast<size_t>(dataLeft[5]) * 8 * 1024;

	// Flags 6
	const uint8_t flags6 = dataLeft[6];
	mirroring = (flags6 & 0x01) ? MirroringType::Vertical : MirroringType::Horizontal;
	hasBatteryRAM = (flags6 & 0x02) != 0;
	const bool hasTrainer = (flags6 & 0x04) != 0;
	const bool ignoreMirroringControl = (flags6 & 0x08) != 0;
	mapper = flags6 >> 4;

	// Flags 7
	const uint8_t flags7 = dataLeft[7];
	const bool vsUnisystem = (flags7 & 0x01) != 0;
	const bool playChoice10 = (flags7 & 0x02) != 0;
	const bool nes2 = ((flags7 & 0x0C) >> 2) == 0x10;
	mapper |= flags7 & 0xF0;

	// Flags 8-15
	// TODO

	// Finish header
	dataLeft = dataLeft.subspan(16);

	auto readBytes = [&] (gsl::span<uint8_t> dst) -> bool
	{
		const size_t nBytes = dst.size();
		if (dataLeft.size() < nBytes) {
			return false;
		}
		memcpy(dst.data(), dataLeft.data(), nBytes);
		dataLeft = dataLeft.subspan(nBytes);

		return true;
	};

	// Load Trainer
	if (hasTrainer) {
		uint8_t buffer[512];
		if (!readBytes(buffer)) {
			return false;
		}
	}

	// PRG ROM
	prgRom.resize(prgRomSize);
	if (!readBytes(prgRom)) {
		prgRom.clear();
		return false;
	}

	// CHR ROM
	chrRom.resize(chrRomSize);
	if (!readBytes(chrRom)) {
		chrRom.clear();
		return false;
	}

	return true;
}
