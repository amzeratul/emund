#pragma once
#include <cstdint>

class AddressSpace8BitBy16Bit;

class CPU6502 {
public:
	enum class ErrorType {
		OK,
		UnknownInstruction
	};
	
	CPU6502();
	
	void setAddressSpace(AddressSpace8BitBy16Bit& addressSpace);
	void tick();

	void reset();

	bool hasError() const;
	ErrorType getError() const;
	uint8_t getErrorInstruction() const;

private:
	AddressSpace8BitBy16Bit* addressSpace = nullptr;

	uint8_t regA = 0;
	uint8_t regX = 0;
	uint8_t regY = 0;
	uint16_t regPC = 0xC000;
	uint8_t regS = 0xFD;
	uint8_t regP = 0x34;

	ErrorType error = ErrorType::OK;
	uint8_t errorInstruction;

	void setZN(uint8_t value);
};
