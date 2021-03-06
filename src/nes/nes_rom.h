#pragma once

#include <gsl/gsl>
#include <cstddef>

class NESRom {
public:
	bool load(gsl::span<const std::byte> romData);
	gsl::span<uint8_t> getPRGROM();
	gsl::span<uint8_t> getCHRROM();
	uint16_t getMapper() const;

private:
	enum class MirroringType : uint8_t {
		Horizontal,
		Vertical
	};

	MirroringType mirroring = MirroringType::Horizontal;
	bool hasBatteryRAM = false;
	uint16_t mapper = 0;

	std::vector<uint8_t> prgRom;
	std::vector<uint8_t> chrRom;
};
