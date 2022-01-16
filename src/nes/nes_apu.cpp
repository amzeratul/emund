#include "nes_apu.h"

void NESAPU::tick()
{
	++cycle;
}

void NESAPU::writeRegister(uint16_t address, uint8_t value)
{
	switch (address) {
	case 0x4000:
		// SQ1_VOL
	    // TODO
	    break;
	case 0x4001:
		// SQ1_SWEEP
	    // TODO
	    break;
	case 0x4002:
		// SQ1_LO
	    // TODO
	    break;
	case 0x4003:
		// SQ1_HI
	    // TODO
	    break;
	case 0x4004:
		// SQ2_VOL
	    // TODO
	    break;
	case 0x4005:
		// SQ2_SWEEP
	    // TODO
	    break;
	case 0x4006:
		// SQ2_LO
	    // TODO
	    break;
	case 0x4007:
		// SQ2_HI
	    // TODO
	    break;
	case 0x4008:
		// TRI_LINEAR
	    // TODO
	    break;
	case 0x4009:
		// 	Unused
	    // TODO
	    break;
	case 0x400A:
		// TRI_LO
	    // TODO
	    break;
	case 0x400B:
		// TRI_HI
	    // TODO
	    break;
	case 0x400C:
		// NOISE_VOL
	    // TODO
	    break;
	case 0x400D:
		// 	Unused
	    // TODO
	    break;
	case 0x400E:
		// NOISE_LO
	    // TODO
	    break;
	case 0x400F:
		// NOISE_HI
	    // TODO
	    break;
	case 0x4010:
		// DMC_FREQ
	    // TODO
	    break;
	case 0x4011:
		// DMC_RAW
	    // TODO
	    break;
	case 0x4012:
		// DMC_START
	    // TODO
	    break;
	case 0x4013:
		// DMC_LEN
	    // TODO
	    break;
	case 0x4015:
		// Control/status
	    // TODO
	    break;
	case 0x4017:
		// Frame counter
		break;
	}
}

uint8_t NESAPU::readRegister(uint16_t address)
{
	if (address == 0x4015) {
		// SND_CHN
		// TODO
	}

	return 0;
}

uint64_t NESAPU::getCycle() const
{
	return cycle;
}
