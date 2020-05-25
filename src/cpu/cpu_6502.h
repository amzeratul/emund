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
		UnknownInstruction,
		Break
	};
	
	CPU6502();
	
	void setAddressSpace(AddressSpace8BitBy16Bit& addressSpace);
	void printDebugInfo();
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
	uint8_t regP = 0x24; // Should this be 0x34?

	std::unique_ptr<CPU6502Disassembler> disassembler;

	ErrorType error = ErrorType::OK;
	uint8_t errorInstruction;

	FORCEINLINE void setZN(uint8_t value);
	FORCEINLINE void setCarry(uint8_t value);

	FORCEINLINE uint8_t loadImmediate();
	FORCEINLINE uint16_t loadImmediate16();
	FORCEINLINE uint8_t loadZeroPage();
	FORCEINLINE uint8_t loadAbsolute();
	FORCEINLINE uint8_t loadAddressMode(uint8_t mode);
	FORCEINLINE uint8_t loadAddressModeX(uint8_t mode);

	FORCEINLINE uint16_t getAbsolute();
	FORCEINLINE uint16_t getAbsolutePlus(uint8_t offset);
	FORCEINLINE uint16_t getZeroPage();
	FORCEINLINE uint16_t getZeroPagePlus(uint8_t offset);
	FORCEINLINE uint16_t getIndirectX();
	FORCEINLINE uint16_t getIndirectY();
	FORCEINLINE uint16_t getAddress(uint8_t mode);
	FORCEINLINE uint16_t getAddressX(uint8_t mode);

	FORCEINLINE void storeAddressMode(uint8_t value, uint8_t mode);
	FORCEINLINE void storeAddressModeX(uint8_t value, uint8_t mode);

	FORCEINLINE void storeStack(uint8_t value);
	FORCEINLINE uint8_t loadStack();
	
	FORCEINLINE void compare(uint8_t reg, uint8_t memory);
	FORCEINLINE void bitTest(uint8_t value);
	FORCEINLINE void addWithCarry(uint8_t value);
	FORCEINLINE void subWithCarry(uint8_t value);
};
