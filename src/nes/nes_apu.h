#pragma once
#include <inttypes.h>

class NESAPU {
public:
	void tick();
	
	void writeRegister(uint16_t address, uint8_t value);
	uint8_t readRegister(uint16_t address);

	uint64_t getCycle() const;

private:
	uint64_t cycle = 0;
};
