#include "cpu_6502.h"

#include "address_space.h"

#include <halley.hpp>
using namespace Halley;

constexpr uint8_t FLAG_CARRY = 0x01;
constexpr uint8_t FLAG_ZERO = 0x02;
constexpr uint8_t FLAG_INTERRUPT_DISABLE = 0x04;
constexpr uint8_t FLAG_DECIMAL = 0x8;
constexpr uint8_t FLAG_B0 = 0x10;
constexpr uint8_t FLAG_B1 = 0x20;
constexpr uint8_t FLAG_OVERFLOW = 0x40;
constexpr uint8_t FLAG_NEGATIVE = 0x80;
constexpr uint8_t FLAG_ALL = 0xFF;

#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif

CPU6502::CPU6502()
{
	disassembler = std::make_unique<CPU6502Disassembler>();
}

void CPU6502::setAddressSpace(AddressSpace8BitBy16Bit& addressSpace)
{
	this->addressSpace = &addressSpace;
}

void CPU6502::tick()
{
	constexpr bool logging = true;
	if (logging) {
		char bufferA[128];
		char bufferB[128];

		size_t n = disassembler->disassemble(addressSpace->read(regPC), addressSpace->read(regPC + 1), addressSpace->read(regPC + 2), bufferA);
		std::snprintf(bufferB, 128, "%04hX                                            A:%02hhX X:%02hhX Y:%02hhX P:%02hhX SP:%02hhX", regPC, regA, regX, regY, regP, regS);
		memcpy(bufferB + 6, bufferA, std::min(strlen(bufferA), size_t(40)));
		
		Logger::logDev(String(bufferB));
	}
	
	const auto instruction = loadImmediate();
	const uint8_t addressMode = (instruction & 0x1C) >> 2;

	switch (instruction) {
	case 0x61:
	case 0x65:
	case 0x69:
	case 0x6D:
	case 0x71:
	case 0x75:
	case 0x79:
	case 0x7D:
		// ADC
		addWithCarry(loadAddressMode(addressMode));
		break;
	case 0x21:
	case 0x25:
	case 0x29:
	case 0x2D:
	case 0x31:
	case 0x35:
	case 0x39:
	case 0x3D:
		// AND
		regA = regA & loadAddressMode(addressMode);
		setZN(regA);
		break;
	case 0x0A:
		// ASL, accumulator
		setCarry(regA & 0x80);
		regA <<= 1;
		setZN(regA);
		break;
	case 0x06:
	case 0x0E:
	case 0x16:
	case 0x1E:
		// ASL
		{
			const auto address = getAddress(addressMode);
			auto m = addressSpace->read(address);
			setCarry(m & 0x80);
			m <<= 1;
			setZN(m);
			addressSpace->write(address, m);
			break;
		}
	case 0x90:
		// BCC
		if (const auto offset = static_cast<int8_t>(loadImmediate()); (regP & FLAG_CARRY) == 0) {
			regPC += offset;
		}
		break;
	case 0xB0:
		// BCS
		if (const auto offset = static_cast<int8_t>(loadImmediate()); (regP & FLAG_CARRY) != 0) {
			regPC += offset;
		}
		break;
	case 0x24:
		// BIT, Zero Page
		bitTest(loadZeroPage());
		break;
	case 0x2C:
		// BIT, Absolute
		bitTest(loadAbsolute());
		break;
	case 0xF0:
		// BEQ
		if (const auto offset = static_cast<int8_t>(loadImmediate()); (regP & FLAG_ZERO) != 0) {
			regPC += offset;
		}
		break;
	case 0x30:
		// BMI
		if (const auto offset = static_cast<int8_t>(loadImmediate()); (regP & FLAG_NEGATIVE) != 0) {
			regPC += offset;
		}
		break;
	case 0xD0:
		// BNE, Relative
		if (const auto offset = static_cast<int8_t>(loadImmediate()); (regP & FLAG_ZERO) == 0) {
			regPC += offset;
		}
		break;
	case 0x10:
		// BPL
		if (const auto offset = static_cast<int8_t>(loadImmediate()); (regP & FLAG_NEGATIVE) == 0) {
			regPC += offset;
		}
		break;
	case 0x00:
		// BRK
		storeStack(regPC >> 8);
		storeStack(regPC & 0xFF);
		storeStack(regP | FLAG_B0 | FLAG_B1);
		regP |= FLAG_INTERRUPT_DISABLE;
		error = ErrorType::Break;
		break;
	case 0x50:
		// BVC
		if (const auto offset = static_cast<int8_t>(loadImmediate()); (regP & FLAG_OVERFLOW) == 0) {
			regPC += offset;
		}
		break;
	case 0x70:
		// BVS
		if (const auto offset = static_cast<int8_t>(loadImmediate()); (regP & FLAG_OVERFLOW) != 0) {
			regPC += offset;
		}
		break;
	case 0x18:
		// CLC
		regP &= ~FLAG_CARRY;
		break;
	case 0xD8:
		// CLD
		regP &= ~FLAG_DECIMAL;
		break;
	case 0x58:
		// CLI
		regP &= ~FLAG_INTERRUPT_DISABLE;
		break;
	case 0xB8:
		// CLV
		regP &= ~FLAG_OVERFLOW;
		break;
	case 0xC1:
	case 0xC5:
	case 0xC9:
	case 0xCD:
	case 0xD1:
	case 0xD5:
	case 0xD9:
	case 0xDD:
		// CMP
		compare(regA, loadAddressMode(addressMode));
		break;
	case 0xE0:
		// CPX
		compare(regX, loadImmediate());
		break;
	case 0xE4:
	case 0xEC:
		// CPX
		compare(regX, loadAddressMode(addressMode));
		break;
	case 0xC0:
		// CPY
		compare(regY, loadImmediate());
		break;
	case 0xC4:
	case 0xCC:
		// CPY
		compare(regY, loadAddressMode(addressMode));
		break;
	case 0xC6:
	case 0xCE:
	case 0xD6:
	case 0xDE:
		// DEC
		{
			const auto m = loadAddressMode(addressMode) - 1;
			setZN(m);
			storeAddressMode(m, addressMode);
			break;
		}
	case 0xCA:
		// DEX
		--regX;
		setZN(regX);
		break;
	case 0x88:
		// DEY
		--regY;
		setZN(regY);
		break;
	case 0x41:
	case 0x45:
	case 0x49:
	case 0x4D:
	case 0x51:
	case 0x55:
	case 0x59:
	case 0x5D:
		// EOR
		regA = regA ^ loadAddressMode(addressMode);
		setZN(regA);
		break;
	case 0xE6:
	case 0xEE:
	case 0xF6:
	case 0xFE:
		// INC
		{
			const auto m = loadAddressMode(addressMode) + 1;
			setZN(m);
			storeAddressMode(m, addressMode);
			break;
		}
	case 0xE8:
		// INX
		++regX;
		setZN(regX);
		break;
	case 0xC8:
		// INY
		++regY;
		setZN(regY);
		break;
	case 0x4C:
		// JMP, immediate
		regPC = loadImmediate16();
		break;
	case 0x6C:
		{
			// JMP, indirect
			const auto addr0 = loadImmediate16();
			const auto lowAddr1 = addressSpace->read(addr0);
			const auto highAddr1 = addressSpace->read(addr0 + 1);
			const auto addr1 = static_cast<uint16_t>(lowAddr1) | (static_cast<uint16_t>(highAddr1) << 8);
			regPC = addr1;
			break;
		}
	case 0x20:
		// JSR
		{
			const auto addr = loadImmediate16();
			--regPC;
			storeStack(regPC >> 8);
			storeStack(regPC & 0xFF);
			regPC = addr;
			break;
		}
	case 0xA5:
	case 0xA9:
	case 0xAD:
	case 0xA1:
	case 0xB1:
	case 0xB5:
	case 0xB9:
	case 0xBD:
		// LDA
		regA = loadAddressMode(addressMode);
		setZN(regA);
		break;
	case 0xA2:
		// LDX, immediate mode
		regX = loadImmediate();
		setZN(regX);
		break;
	case 0xA6:
	case 0xAE:
	case 0xB6:
	case 0xBE:
		// LDX
		regX = loadAddressModeX(addressMode);
		setZN(regX);
		break;
	case 0xA0:
		// LDY, immediate mode
		regY = loadImmediate();
		setZN(regY);
		break;
	case 0xA4:
	case 0xAC:
	case 0xB4:
	case 0xBC:
		// LDY
		regY = loadAddressMode(addressMode);
		setZN(regY);
		break;
	case 0x4A:
		// LSR, accumulator
		setCarry(regA & 1);
		regA >>= 1;
		setZN(regA);
		break;
	case 0x46:
	case 0x4E:
	case 0x56:
	case 0x5E:
		// LSR
		{
			const auto address = getAddress(addressMode);
			auto m = addressSpace->read(address);
			setCarry(m & 1);
			m >>= 1;
			setZN(m);
			addressSpace->write(address, m);
			break;
		}
	case 0xEA:
		// NOP
		break;
	case 0x01:
	case 0x05:
	case 0x09:
	case 0x0D:
	case 0x11:
	case 0x15:
	case 0x19:
	case 0x1D:
		// ORA
		regA = regA | loadAddressMode(addressMode);
		setZN(regA);
		break;
	case 0x08:
		// PHP
		storeStack(regP | FLAG_B0 | FLAG_B1);
		break;
	case 0x48:
		// PHA
		storeStack(regA);
		break;
	case 0x68:
		// PLA
		regA = loadStack();
		setZN(regA);
		break;
	case 0x28:
		// PLP
		regP = (loadStack() & ~(FLAG_B0)) | FLAG_B1;
		// This really should have B0 and B1 disabled I think?
		break;
	case 0x2A:
		// ROL, accumulator
		{
			const uint8_t carry = (regP & FLAG_CARRY);
			const auto address = getAddress(addressMode);
			auto m = addressSpace->read(address);
			setCarry(regA & 0x80);
			regA = (regA << 1) | carry;
			setZN(regA);
			addressSpace->write(address, m);
			break;
		}
	case 0x26:
	case 0x2E:
	case 0x36:
	case 0x3E:
		// ROL
		{
			const auto address = getAddress(addressMode);
			auto m = addressSpace->read(address);
			const uint8_t carry = (regP & FLAG_CARRY);
			setCarry(m & 0x80);
			m = (m << 1) | carry;
			setZN(m);
			addressSpace->write(address, m);
			break;
		}
	case 0x6A:
		// ROR, accumulator
		{
			const uint8_t carry = (regP & FLAG_CARRY);
			setCarry(regA & 1);
			regA = (regA >> 1) | (carry << 7);
			setZN(regA);
			break;
		}
	case 0x66:
	case 0x6E:
	case 0x76:
	case 0x7E:
		// ROR
		{
			const auto address = getAddress(addressMode);
			auto m = addressSpace->read(address);
			const uint8_t carry = (regP & FLAG_CARRY);
			setCarry(m & 1);
			m = (m >> 1) | (carry << 7);
			setZN(m);
			addressSpace->write(address, m);
			break;
		}
	case 0x40:
		// RTI
		regP = (loadStack() & ~(FLAG_B0)) | FLAG_B1;
		// This really should have B0 and B1 disabled I think?
		regPC = loadStack();
		regPC |= uint16_t(loadStack()) << 8;
		break;
	case 0x60:
		// RTS
		regPC = loadStack();
		regPC |= uint16_t(loadStack()) << 8;
		++regPC;
		break;
	case 0xE1:
	case 0xE5:
	case 0xE9:
	case 0xED:
	case 0xF1:
	case 0xF5:
	case 0xF9:
	case 0xFD:
		// SBC
		subWithCarry(loadAddressMode(addressMode));
		break;
	case 0x38:
		// SEC
		regP |= FLAG_CARRY;
		break;
	case 0xF8:
		// SED
		regP |= FLAG_DECIMAL;
		break;
	case 0x78:
		// SEI
		regP |= FLAG_INTERRUPT_DISABLE;
		break;
	case 0x81:
	case 0x85:
	case 0x8D:
	case 0x91:
	case 0x95:
	case 0x99:
	case 0x9D:
		// STA
		storeAddressMode(regA, addressMode);
		break;
	case 0x86:
	case 0x8E:
	case 0x96:
		// STX
		storeAddressModeX(regX, addressMode);
		break;
	case 0x84:
	case 0x8C:
	case 0x94:
		// STY
		storeAddressMode(regY, addressMode);
		break;
	case 0xAA:
		// TAX
		regX = regA;
		setZN(regX);
		break;
	case 0xA8:
		// TAY
		regY = regA;
		setZN(regY);
		break;
	case 0xBA:
		// TSX
		regX = regS;
		setZN(regX);
		break;
	case 0x8A:
		// TXA
		regA = regX;
		setZN(regA);
		break;
	case 0x9A:
		// TXS
		regS = regX;
		break;
	case 0x98:
		// TYA
		regA = regY;
		setZN(regA);
		break;
	default:
		error = ErrorType::UnknownInstruction;
		errorInstruction = instruction;
	}
}

void CPU6502::reset()
{
	regS -= 3;
	regP |= 0x04;
}

bool CPU6502::hasError() const
{
	return error != ErrorType::OK;
}

CPU6502::ErrorType CPU6502::getError() const
{
	return error;
}

uint8_t CPU6502::getErrorInstruction() const
{
	return errorInstruction;
}

void CPU6502::setZN(uint8_t value)
{
	regP = (regP & ~(FLAG_ZERO | FLAG_NEGATIVE)) | (value == 0 ? FLAG_ZERO : 0) | (value & 0x80 ? FLAG_NEGATIVE : 0);
}

void CPU6502::setCarry(uint8_t value)
{
	regP = (regP & ~FLAG_CARRY) | (value ? FLAG_CARRY : 0);
}

uint8_t CPU6502::loadImmediate()
{
	return addressSpace->read(regPC++);
}

uint16_t CPU6502::loadImmediate16()
{
	const auto lowAddr = loadImmediate();
	const auto highAddr = loadImmediate();
	return static_cast<uint16_t>(lowAddr) | (static_cast<uint16_t>(highAddr) << 8);
}

uint8_t CPU6502::loadZeroPage()
{
	return addressSpace->read(getZeroPage());
}

uint8_t CPU6502::loadAbsolute()
{
	return addressSpace->read(getAbsolute());
}

uint8_t CPU6502::loadAddressMode(uint8_t mode)
{
	if (mode == 2) {
		return loadImmediate();
	}
	return addressSpace->read(getAddress(mode));
}

uint8_t CPU6502::loadAddressModeX(uint8_t mode)
{
	if (mode == 2) {
		return loadImmediate();
	}
	return addressSpace->read(getAddress(mode));
}

uint16_t CPU6502::getAbsolute()
{
	const auto lowAddr = loadImmediate();
	const auto highAddr = loadImmediate();
	return static_cast<uint16_t>(lowAddr) | (static_cast<uint16_t>(highAddr) << 8);
}

uint16_t CPU6502::getAbsolutePlus(uint8_t offset)
{
	const auto lowAddr = loadImmediate();
	const auto highAddr = loadImmediate();
	const auto addr = static_cast<uint16_t>(lowAddr) | (static_cast<uint16_t>(highAddr) << 8);
	return addr + offset;
}

uint16_t CPU6502::getZeroPage()
{
	return loadImmediate();
}

uint16_t CPU6502::getZeroPagePlus(uint8_t offset)
{
	const auto lowAddr = loadImmediate();
	return static_cast<uint16_t>(lowAddr + offset);
}

uint16_t CPU6502::getIndirectX()
{
	const uint8_t immediate = loadImmediate();
	const auto lowAddr = addressSpace->read(uint8_t(immediate + regX));
	const auto highAddr = addressSpace->read(uint8_t(immediate + regX + 1));
	return static_cast<uint16_t>(lowAddr) | (static_cast<uint16_t>(highAddr) << 8);
}

uint16_t CPU6502::getIndirectY()
{	const auto tablePos = loadImmediate16();
	const auto lowAddr = addressSpace->read(tablePos);
	const auto highAddr = addressSpace->read(tablePos + 1);
	const auto addr = static_cast<uint16_t>(lowAddr) | (static_cast<uint16_t>(highAddr) << 8);
	return addr + regY;
}

uint16_t CPU6502::getAddress(uint8_t mode)
{
	switch (mode) {
	case 0:
		return getIndirectX();
	case 1:
		return getZeroPage();
	case 2:
		// Immediate mode, should never get here
		return 0;
	case 3:
		return getAbsolute();
	case 4:
		return getIndirectY();
	case 5:
		return getZeroPagePlus(regX);
	case 6:
		return getAbsolutePlus(regY);
	default:
		return getAbsolutePlus(regX);
	}
}

uint16_t CPU6502::getAddressX(uint8_t mode)
{
	switch (mode) {
	case 0:
		return getIndirectX();
	case 1:
		return getZeroPage();
	case 2:
		// Immediate mode, should never get here
		return 0;
	case 3:
		return getAbsolute();
	case 4:
		return getIndirectY();
	case 5:
		return getZeroPagePlus(regX);
	case 6:
		return getAbsolutePlus(regY);
	default:
		return getAbsolutePlus(regY);
	}
}

void CPU6502::storeAddressMode(uint8_t value, uint8_t mode)
{
	addressSpace->write(getAddress(mode), value);
}

void CPU6502::storeAddressModeX(uint8_t value, uint8_t mode)
{
	addressSpace->write(getAddressX(mode), value);
}

void CPU6502::storeStack(uint8_t value)
{
	addressSpace->write(0x100 + regS--, value);
}

uint8_t CPU6502::loadStack()
{
	return addressSpace->read(0x100 + ++regS);
}

void CPU6502::compare(uint8_t reg, uint8_t memory)
{
	regP = (regP & ~(FLAG_CARRY | FLAG_ZERO | FLAG_NEGATIVE))
		| (reg >= memory ? FLAG_CARRY : 0)
		| (reg == memory ? FLAG_ZERO : 0)
		| (((reg - memory) & 0x80) != 0 ? FLAG_NEGATIVE : 0);
}

void CPU6502::bitTest(uint8_t value)
{
	regP = (regP & ~(FLAG_ZERO | FLAG_OVERFLOW | FLAG_NEGATIVE)) | ((regA & value) == 0 ? FLAG_ZERO : 0) | (value & (FLAG_OVERFLOW | FLAG_NEGATIVE));
}

void CPU6502::addWithCarry(uint8_t value)
{
	const uint8_t a = regA;
	const uint16_t intermediateResult = static_cast<uint16_t>(a) + static_cast<uint16_t>(value) + static_cast<uint16_t>(regP & FLAG_CARRY);
	
	regA = static_cast<uint8_t>(intermediateResult); // Narrow result

	regP = (regP & ~(FLAG_CARRY | FLAG_ZERO | FLAG_OVERFLOW | FLAG_NEGATIVE))
		| ((intermediateResult & 0x100) >> 8) // Set carry flag
		| (regA == 0 ? FLAG_ZERO : 0) // Set zero flag
		| (((a ^ regA) & (value ^ regA) & 0x80) != 0 ? FLAG_OVERFLOW : 0) // Set overflow flag. See http://www.righto.com/2012/12/the-6502-overflow-flag-explained.html
		| (regA & 0x80); // Set negative flag
}

void CPU6502::subWithCarry(uint8_t value)
{
	const uint8_t a = regA;
	const uint16_t intermediateResult = static_cast<uint16_t>(a) - static_cast<uint16_t>(value) - static_cast<uint16_t>(~regP & FLAG_CARRY);
	
	regA = static_cast<uint8_t>(intermediateResult); // Narrow result

	regP = (regP & ~(FLAG_CARRY | FLAG_ZERO | FLAG_OVERFLOW | FLAG_NEGATIVE))
		| (((intermediateResult & 0x100) >> 8) ^ FLAG_CARRY) // Set carry flag
		| (regA == 0 ? FLAG_ZERO : 0) // Set zero flag
		| (((a ^ regA) & ((255 - value) ^ regA) & 0x80) != 0 ? FLAG_OVERFLOW : 0) // Set overflow flag. See http://www.righto.com/2012/12/the-6502-overflow-flag-explained.html
		| (regA & 0x80); // Set negative flag
}

