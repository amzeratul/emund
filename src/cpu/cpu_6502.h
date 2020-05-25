#pragma once
#include <cstdint>
#include <memory>

#include "cpu_6502_disassembler.h"
#include "../utils/macros.h"

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
	uint8_t regP = 0x24;

	std::unique_ptr<CPU6502Disassembler> disassembler;

	ErrorType error = ErrorType::OK;
	uint8_t errorInstruction;

	void setZN(uint8_t value);

	FORCEINLINE uint8_t loadImmediate();
	FORCEINLINE uint16_t loadImmediate16();
	FORCEINLINE uint8_t loadAbsolute();
	FORCEINLINE uint8_t loadAbsolutePlus(uint8_t offset);
	FORCEINLINE uint8_t loadZeroPage();
	FORCEINLINE uint8_t loadZeroPagePlus(uint8_t offset);
	FORCEINLINE uint8_t loadIndirectX();
	FORCEINLINE uint8_t loadIndirectY();

	FORCEINLINE void storeAbsolute(uint8_t value);
	FORCEINLINE void storeAbsolutePlus(uint8_t value, uint8_t offset);
	FORCEINLINE void storeZeroPage(uint8_t value);
	FORCEINLINE void storeZeroPagePlus(uint8_t value, uint8_t offset);
	FORCEINLINE void storeIndirectX(uint8_t value);
	FORCEINLINE void storeIndirectY(uint8_t value);

	FORCEINLINE void storeStack(uint8_t value);
	FORCEINLINE uint8_t loadStack();
	
	FORCEINLINE void compare(uint8_t value);
	FORCEINLINE void bitTest(uint8_t value);
};
