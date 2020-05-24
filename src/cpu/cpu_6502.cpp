#include "cpu_6502.h"

#include "address_space.h"

constexpr uint8_t FLAG_CARRY = 0x01;
constexpr uint8_t FLAG_ZERO = 0x02;
constexpr uint8_t FLAG_INTERRUPT_DISABLE = 0x04;
constexpr uint8_t FLAG_DECIMAL = 0x8;
constexpr uint8_t FLAG_B = 0x30;
constexpr uint8_t FLAG_OVERFLOW = 0x40;
constexpr uint8_t FLAG_NEGATIVE = 0x80;
constexpr uint8_t FLAG_ALL = 0xFF;

CPU6502::CPU6502()
{
}

void CPU6502::setAddressSpace(AddressSpace8BitBy16Bit& addressSpace)
{
	this->addressSpace = &addressSpace;
}

void CPU6502::tick()
{
	const auto instruction = loadImmediate();
	
	switch (instruction) {
	case 0x18:
		// CLC
		regP &= ~FLAG_CARRY;
		break;
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
	case 0x38:
		// SEC
		regP |= FLAG_CARRY;
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
	case 0x84:
		// STY, zero page
		storeZeroPage(regY);
		break;
	case 0x86:
		// STX, zero page
		storeZeroPage(regX);
		break;
	case 0x8C:
		// STY, absolute
		storeAbsolute(regY);
		break;
	case 0x8E:
		// STX, absolute
		storeAbsolute(regX);
		break;
	case 0x90:
		// BCC
		if (const auto offset = static_cast<int8_t>(loadImmediate()); (regP & FLAG_CARRY) == 0) {
			regPC += offset;
		}
		break;
	case 0x94:
		// STY, zero page + X
		storeZeroPagePlus(regY, regX);
		break;
	case 0x96:
		// STX, zero page + Y
		storeZeroPagePlus(regX, regY);
		break;
	case 0xA0:
		// LDY, immediate
		regY = loadImmediate();
		setZN(regY);
		break;
	case 0xA2:
		// LDX, immediate
		regX = loadImmediate();
		setZN(regX);
		break;
	case 0xA4:
		// LDY, zero page
		regY = loadZeroPage();
		setZN(regY);
		break;
	case 0xA6:
		// LDX, zero page
		regX = loadZeroPage();
		setZN(regX);
		break;
	case 0xAC:
		// LDY, absolute
		regY = loadAbsolute();
		setZN(regY);
		break;
	case 0xAE:
		// LDX, absolute
		regX = loadAbsolute();
		setZN(regX);
		break;
	case 0xB4:
		// LDY, zero page + X
		regY = loadZeroPagePlus(regX);
		setZN(regY);
		break;
	case 0xB0:
		// BCS
		regPC += static_cast<int8_t>(loadImmediate());
		break;
	case 0xB6:
		// LDX, zero page + Y
		regX = loadZeroPagePlus(regY);
		setZN(regX);
		break;
	case 0xBC:
		// LDY, absolute + X
		regY = loadAbsolutePlus(regX);
		setZN(regY);
		break;
	case 0xBE:
		// LDX, absolute + Y
		regX = loadAbsolutePlus(regY);
		setZN(regX);
		break;
	case 0xC1:
		// CMP, (Indirect, X)
		compare(loadIndirectX());
		break;
	case 0xC5:
		// CMP, Zero Page
		compare(loadZeroPage());
		break;
	case 0xC9:
		// CMP, Immediate
		compare(loadImmediate());
		break;
	case 0xCD:
		// CMP, Absolute
		compare(loadAbsolute());
		break;
	case 0xD1:
		// CMP, (Indirect), Y
		compare(loadIndirectY());
		break;
	case 0xD5:
		// CMP, Zero Page + X
		compare(loadZeroPagePlus(regX));
		break;
	case 0xD9:
		// CMP, Absolute + Y
		compare(loadAbsolutePlus(regY));
		break;
	case 0xDD:
		// CMP, Absolute + X
		compare(loadAbsolutePlus(regX));
		break;
	case 0xEA:
		// NOP
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

uint8_t CPU6502::loadImmediate()
{
	return addressSpace->read(regPC++);
};

uint16_t CPU6502::loadImmediate16()
{
	const auto lowAddr = loadImmediate();
	const auto highAddr = loadImmediate();
	return static_cast<uint16_t>(lowAddr) | (static_cast<uint16_t>(highAddr) << 8);
};

uint8_t CPU6502::loadAbsolute()
{
	const auto lowAddr = loadImmediate();
	const auto highAddr = loadImmediate();
	const auto addr =  static_cast<uint16_t>(lowAddr) | (static_cast<uint16_t>(highAddr) << 8);
	return addressSpace->read(addr);
};

uint8_t CPU6502::loadAbsolutePlus(uint8_t offset)
{
	const auto lowAddr = loadImmediate();
	const auto highAddr = loadImmediate();
	const auto addr =  (static_cast<uint16_t>(lowAddr) | (static_cast<uint16_t>(highAddr) << 8)) + offset;
	return addressSpace->read(addr);
};

uint8_t CPU6502::loadZeroPage()
{
	const auto lowAddr = loadImmediate();
	return addressSpace->read(static_cast<uint16_t>(lowAddr));
};

uint8_t CPU6502::loadZeroPagePlus(uint8_t offset)
{
	const auto lowAddr = loadImmediate();
	return addressSpace->read(static_cast<uint16_t>(lowAddr) + offset);
};

uint8_t CPU6502::loadIndirectX()
{
	const auto tablePos = loadImmediate();
	return addressSpace->read(static_cast<uint16_t>(tablePos + regX));
};

uint8_t CPU6502::loadIndirectY()
{
	const auto addr = loadImmediate16();
	return addressSpace->read(addr + regY);
};

void CPU6502::storeAbsolute(uint8_t value)
{
	const auto lowAddr = loadImmediate();
	const auto highAddr = loadImmediate();
	const auto addr =  static_cast<uint16_t>(lowAddr) | (static_cast<uint16_t>(highAddr) << 8);
	addressSpace->write(addr, value);
};

void CPU6502::storeZeroPage(uint8_t value)
{
	const auto lowAddr = loadImmediate();
	addressSpace->write(static_cast<uint16_t>(lowAddr), value);
};

void CPU6502::storeZeroPagePlus(uint8_t value, uint8_t offset)
{
	const auto lowAddr = loadImmediate();
	addressSpace->write(static_cast<uint16_t>(lowAddr) + offset, value);
};

void CPU6502::storeStack(uint8_t value)
{
	addressSpace->write(0x100 + regS--, value);
};

void CPU6502::compare(uint8_t value)
{
	const auto temp = value - regA;
	regP = (regP & ~(FLAG_CARRY | FLAG_ZERO | FLAG_NEGATIVE)) | (regA >= value ? FLAG_CARRY : 0) | (regA == value ? FLAG_CARRY : 0) | ((temp & 0x80) != 0 ? FLAG_NEGATIVE : 0);
};
