#include "cpu_6502_disassembler.h"

#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif

CPU6502Disassembler::CPU6502Disassembler()
{
	entries.resize(256);
	addEntry(0x69, "ADC", "#$44"    , "Immediate"  , 2, "2");
	addEntry(0x65, "ADC", "$44"     , "Zero Page"  , 2, "3");
	addEntry(0x75, "ADC", "$44,X"   , "Zero Page,X", 2, "4");
	addEntry(0x6D, "ADC", "$4400"   , "Absolute"   , 3, "4");
	addEntry(0x7D, "ADC", "$4400,X" , "Absolute,X" , 3, "4+");
	addEntry(0x79, "ADC", "$4400,Y" , "Absolute,Y" , 3, "4+");
	addEntry(0x61, "ADC", "($44,X)" , "Indirect,X" , 2, "6");
	addEntry(0x71, "ADC", "($44),Y" , "Indirect,Y" , 2, "5+");
	addEntry(0x29, "AND", "#$44"    , "Immediate"  , 2, "2");
	addEntry(0x25, "AND", "$44"     , "Zero Page"  , 2, "3");
	addEntry(0x35, "AND", "$44,X"   , "Zero Page,X", 2, "4");
	addEntry(0x2D, "AND", "$4400"   , "Absolute"   , 3, "4");
	addEntry(0x3D, "AND", "$4400,X" , "Absolute,X" , 3, "4+");
	addEntry(0x39, "AND", "$4400,Y" , "Absolute,Y" , 3, "4+");
	addEntry(0x21, "AND", "($44,X)" , "Indirect,X" , 2, "6");
	addEntry(0x31, "AND", "($44),Y" , "Indirect,Y" , 2, "5+");
	addEntry(0x0A, "ASL", "A"       , "Accumulator", 1, "2");
	addEntry(0x06, "ASL", "$44"     , "Zero Page"  , 2, "5");
	addEntry(0x16, "ASL", "$44,X"   , "Zero Page,X", 2, "6");
	addEntry(0x0E, "ASL", "$4400"   , "Absolute"   , 3, "6");
	addEntry(0x1E, "ASL", "$4400,X" , "Absolute,X" , 3, "7");
	addEntry(0x24, "BIT", "$44"     , "Zero Page"  , 2, "3");
	addEntry(0x2C, "BIT", "$4400"   , "Absolute"   , 3, "4");
	addEntry(0x00, "BRK", ""        , "Implied"    , 1, "7");
	addEntry(0xC9, "CMP", "#$44"    , "Immediate"  , 2, "2");
	addEntry(0xC5, "CMP", "$44"     , "Zero Page"  , 2, "3");
	addEntry(0xD5, "CMP", "$44,X"   , "Zero Page,X", 2, "4");
	addEntry(0xCD, "CMP", "$4400"   , "Absolute"   , 3, "4");
	addEntry(0xDD, "CMP", "$4400,X" , "Absolute,X" , 3, "4+");
	addEntry(0xD9, "CMP", "$4400,Y" , "Absolute,Y" , 3, "4+");
	addEntry(0xC1, "CMP", "($44,X)" , "Indirect,X" , 2, "6");
	addEntry(0xD1, "CMP", "($44),Y" , "Indirect,Y" , 2, "5+");
	addEntry(0xE0, "CPX", "#$44"    , "Immediate"  , 2, "2");
	addEntry(0xE4, "CPX", "$44"     , "Zero Page"  , 2, "3");
	addEntry(0xEC, "CPX", "$4400"   , "Absolute"   , 3, "4");
	addEntry(0xC0, "CPY", "#$44"    , "Immediate"  , 2, "2");
	addEntry(0xC4, "CPY", "$44"     , "Zero Page"  , 2, "3");
	addEntry(0xCC, "CPY", "$4400"   , "Absolute"   , 3, "4");
	addEntry(0xC6, "DEC", "$44"     , "Zero Page"  , 2, "5");
	addEntry(0xD6, "DEC", "$44,X"   , "Zero Page,X", 2, "6");
	addEntry(0xCE, "DEC", "$4400"   , "Absolute"   , 3, "6");
	addEntry(0xDE, "DEC", "$4400,X" , "Absolute,X" , 3, "7");
	addEntry(0x49, "EOR", "#$44"    , "Immediate"  , 2, "2");
	addEntry(0x45, "EOR", "$44"     , "Zero Page"  , 2, "3");
	addEntry(0x55, "EOR", "$44,X"   , "Zero Page,X", 2, "4");
	addEntry(0x4D, "EOR", "$4400"   , "Absolute"   , 3, "4");
	addEntry(0x5D, "EOR", "$4400,X" , "Absolute,X" , 3, "4+");
	addEntry(0x59, "EOR", "$4400,Y" , "Absolute,Y" , 3, "4+");
	addEntry(0x41, "EOR", "($44,X)" , "Indirect,X" , 2, "6");
	addEntry(0x51, "EOR", "($44),Y" , "Indirect,Y" , 2, "5+");
	addEntry(0xE6, "INC", "$44"     , "Zero Page"  , 2, "5");
	addEntry(0xF6, "INC", "$44,X"   , "Zero Page,X", 2, "6");
	addEntry(0xEE, "INC", "$4400"   , "Absolute"   , 3, "6");
	addEntry(0xFE, "INC", "$4400,X" , "Absolute,X" , 3, "7");
	addEntry(0x4C, "JMP", "$5597"   , "Absolute"   , 3, "3");
	addEntry(0x6C, "JMP", "($5597)" , "Indirect"   , 3, "5");
	addEntry(0x20, "JSR", "$5597"   , "Absolute"   , 3, "6");
	addEntry(0xA9, "LDA", "#$44"    , "Immediate"  , 2, "2");
	addEntry(0xA5, "LDA", "$44"     , "Zero Page"  , 2, "3");
	addEntry(0xB5, "LDA", "$44,X"   , "Zero Page,X", 2, "4");
	addEntry(0xAD, "LDA", "$4400"   , "Absolute"   , 3, "4");
	addEntry(0xBD, "LDA", "$4400,X" , "Absolute,X" , 3, "4+");
	addEntry(0xB9, "LDA", "$4400,Y" , "Absolute,Y" , 3, "4+");
	addEntry(0xA1, "LDA", "($44,X)" , "Indirect,X" , 2, "6");
	addEntry(0xB1, "LDA", "($44),Y" , "Indirect,Y" , 2, "5+");
	addEntry(0xA2, "LDX", "#$44"    , "Immediate"  , 2, "2");
	addEntry(0xA6, "LDX", "$44"     , "Zero Page"  , 2, "3");
	addEntry(0xB6, "LDX", "$44,Y"   , "Zero Page,Y", 2, "4");
	addEntry(0xAE, "LDX", "$4400"   , "Absolute"   , 3, "4");
	addEntry(0xBE, "LDX", "$4400,Y" , "Absolute,Y" , 3, "4+");
	addEntry(0xA0, "LDY", "#$44"    , "Immediate"  , 2, "2");
	addEntry(0xA4, "LDY", "$44"     , "Zero Page"  , 2, "3");
	addEntry(0xB4, "LDY", "$44,X"   , "Zero Page,X", 2, "4");
	addEntry(0xAC, "LDY", "$4400"   , "Absolute"   , 3, "4");
	addEntry(0xBC, "LDY", "$4400,X" , "Absolute,X" , 3, "4+");
	addEntry(0x4A, "LSR", "A"       , "Accumulator", 1, "2");
	addEntry(0x46, "LSR", "$44"     , "Zero Page"  , 2, "5");
	addEntry(0x56, "LSR", "$44,X"   , "Zero Page,X", 2, "6");
	addEntry(0x4E, "LSR", "$4400"   , "Absolute"   , 3, "6");
	addEntry(0x5E, "LSR", "$4400,X" , "Absolute,X" , 3, "7");
	addEntry(0xEA, "NOP", ""        , "Implied"    , 1, "2");
	addEntry(0x09, "ORA", "#$44"    , "Immediate"  , 2, "2");
	addEntry(0x05, "ORA", "$44"     , "Zero Page"  , 2, "3");
	addEntry(0x15, "ORA", "$44,X"   , "Zero Page,X", 2, "4");
	addEntry(0x0D, "ORA", "$4400"   , "Absolute"   , 3, "4");
	addEntry(0x1D, "ORA", "$4400,X" , "Absolute,X" , 3, "4+");
	addEntry(0x19, "ORA", "$4400,Y" , "Absolute,Y" , 3, "4+");
	addEntry(0x01, "ORA", "($44,X)" , "Indirect,X" , 2, "6");
	addEntry(0x11, "ORA", "($44),Y" , "Indirect,Y" , 2, "5+");
	addEntry(0x2A, "ROL", "A"       , "Accumulator", 1, "2");
	addEntry(0x26, "ROL", "$44"     , "Zero Page"  , 2, "5");
	addEntry(0x36, "ROL", "$44,X"   , "Zero Page,X", 2, "6");
	addEntry(0x2E, "ROL", "$4400"   , "Absolute"   , 3, "6");
	addEntry(0x3E, "ROL", "$4400,X" , "Absolute,X" , 3, "7");
	addEntry(0x6A, "ROR", "A"       , "Accumulator", 1, "2");
	addEntry(0x66, "ROR", "$44"     , "Zero Page"  , 2, "5");
	addEntry(0x76, "ROR", "$44,X"   , "Zero Page,X", 2, "6");
	addEntry(0x6E, "ROR", "$4400"   , "Absolute"   , 3, "6");
	addEntry(0x7E, "ROR", "$4400,X" , "Absolute,X" , 3, "7");
	addEntry(0x40, "RTI", ""        , "Implied"    , 1, "6");
	addEntry(0x60, "RTS", ""        , "Implied"    , 1, "6");
	addEntry(0xE9, "SBC", "#$44"    , "Immediate"  , 2, "2");
	addEntry(0xE5, "SBC", "$44"     , "Zero Page"  , 2, "3");
	addEntry(0xF5, "SBC", "$44,X"   , "Zero Page,X", 2, "4");
	addEntry(0xED, "SBC", "$4400"   , "Absolute"   , 3, "4");
	addEntry(0xFD, "SBC", "$4400,X" , "Absolute,X" , 3, "4+");
	addEntry(0xF9, "SBC", "$4400,Y" , "Absolute,Y" , 3, "4+");
	addEntry(0xE1, "SBC", "($44,X)" , "Indirect,X" , 2, "6");
	addEntry(0xF1, "SBC", "($44),Y" , "Indirect,Y" , 2, "5+");
	addEntry(0x85, "STA", "$44"     , "Zero Page"  , 2, "3");
	addEntry(0x95, "STA", "$44,X"   , "Zero Page,X", 2, "4");
	addEntry(0x8D, "STA", "$4400"   , "Absolute"   , 3, "4");
	addEntry(0x9D, "STA", "$4400,X" , "Absolute,X" , 3, "5");
	addEntry(0x99, "STA", "$4400,Y" , "Absolute,Y" , 3, "5");
	addEntry(0x81, "STA", "($44,X)" , "Indirect,X" , 2, "6");
	addEntry(0x91, "STA", "($44),Y" , "Indirect,Y" , 2, "6");
	addEntry(0x86, "STX", "$44"     , "Zero Page"  , 2, "3");
	addEntry(0x96, "STX", "$44,Y"   , "Zero Page,Y", 2, "4");
	addEntry(0x8E, "STX", "$4400"   , "Absolute"   , 3, "4");
	addEntry(0x84, "STY", "$44"     , "Zero Page"  , 2, "3");
	addEntry(0x94, "STY", "$44,X"   , "Zero Page,X", 2, "4");
	addEntry(0x8C, "STY", "$4400"   , "Absolute"   , 3, "4");
	addEntry(0x10, "BPL", ""        , "Relative"   , 2, "2++");
	addEntry(0x30, "BMI", ""        , "Relative"   , 2, "2++");
	addEntry(0x50, "BVC", ""        , "Relative"   , 2, "2++");
	addEntry(0x70, "BVS", ""        , "Relative"   , 2, "2++");
	addEntry(0x90, "BCC", ""        , "Relative"   , 2, "2++");
	addEntry(0xB0, "BCS", ""        , "Relative"   , 2, "2++");
	addEntry(0xD0, "BNE", ""        , "Relative"   , 2, "2++");
	addEntry(0xF0, "BEQ", ""        , "Relative"   , 2, "2++");
	addEntry(0x18, "CLC", ""        , "Implied"    , 1, "2");
	addEntry(0x38, "SEC", ""        , "Implied"    , 1, "2");
	addEntry(0x58, "CLI", ""        , "Implied"    , 1, "2");
	addEntry(0x78, "SEI", ""        , "Implied"    , 1, "2");
	addEntry(0xB8, "CLV", ""        , "Implied"    , 1, "2");
	addEntry(0xD8, "CLD", ""        , "Implied"    , 1, "2");
	addEntry(0xF8, "SED", ""        , "Implied"    , 1, "2");
	addEntry(0x9A, "TXS", ""        , "Implied"    , 1, "2");
	addEntry(0xBA, "TSX", ""        , "Implied"    , 1, "2");
	addEntry(0x48, "PHA", ""        , "Implied"    , 1, "3");
	addEntry(0x68, "PLA", ""        , "Implied"    , 1, "4");
	addEntry(0x08, "PHP", ""        , "Implied"    , 1, "3");
	addEntry(0x28, "PLP", ""        , "Implied"    , 1, "4");
	addEntry(0xAA, "TAX", ""        , "Implied"    , 1, "2");
	addEntry(0x8A, "TXA", ""        , "Implied"    , 1, "2");
	addEntry(0xCA, "DEX", ""        , "Implied"    , 1, "2");
	addEntry(0xE8, "INX", ""        , "Implied"    , 1, "2");
	addEntry(0xA8, "TAY", ""        , "Implied"    , 1, "2");
	addEntry(0x98, "TYA", ""        , "Implied"    , 1, "2");
	addEntry(0x88, "DEY", ""        , "Implied"    , 1, "2");
	addEntry(0xC8, "INY", ""        , "Implied"    , 1, "2");

	addEntry(0x1A, "NOP", ""        , "Implied"    , 1, "1", true);
	addEntry(0x3A, "NOP", ""        , "Implied"    , 1, "1", true);
	addEntry(0x5A, "NOP", ""        , "Implied"    , 1, "1", true);
	addEntry(0x7A, "NOP", ""        , "Implied"    , 1, "1", true);
	addEntry(0xDA, "NOP", ""        , "Implied"    , 1, "1", true);
	addEntry(0xFA, "NOP", ""        , "Implied"    , 1, "1", true);

	addEntry(0x04, "NOP", ""        , "Implied"    , 2, "1", true);
	addEntry(0x44, "NOP", ""        , "Implied"    , 2, "1", true);
	addEntry(0x64, "NOP", ""        , "Implied"    , 2, "1", true);
	addEntry(0x14, "NOP", ""        , "Implied"    , 2, "1", true);
	addEntry(0x34, "NOP", ""        , "Implied"    , 2, "1", true);
	addEntry(0x44, "NOP", ""        , "Implied"    , 2, "1", true);
	addEntry(0x54, "NOP", ""        , "Implied"    , 2, "1", true);
	addEntry(0x64, "NOP", ""        , "Implied"    , 2, "1", true);
	addEntry(0x74, "NOP", ""        , "Implied"    , 2, "1", true);
	addEntry(0x80, "NOP", ""        , "Implied"    , 2, "1", true);
	addEntry(0xD4, "NOP", ""        , "Implied"    , 2, "1", true);
	addEntry(0xF4, "NOP", ""        , "Implied"    , 2, "1", true);

	addEntry(0x0C, "NOP", ""        , "Implied"    , 3, "1", true);
	addEntry(0x1C, "NOP", ""        , "Implied"    , 3, "1", true);
	addEntry(0x3C, "NOP", ""        , "Implied"    , 3, "1", true);
	addEntry(0x5C, "NOP", ""        , "Implied"    , 3, "1", true);
	addEntry(0x7C, "NOP", ""        , "Implied"    , 3, "1", true);
	addEntry(0xDC, "NOP", ""        , "Implied"    , 3, "1", true);
	addEntry(0xFC, "NOP", ""        , "Implied"    , 3, "1", true);

	addEntry(0xA7, "LAX", "$44"     , "Zero Page"  , 2, "3", true);
	addEntry(0xB7, "LAX", "$44,X"   , "Zero Page,X", 2, "4", true);
	addEntry(0xAF, "LAX", "$4400"   , "Absolute"   , 3, "4", true);
	addEntry(0xBF, "LAX", "$4400,X" , "Absolute,X" , 3, "4+", true);
	addEntry(0xA3, "LAX", "($44,X)" , "Indirect,X" , 2, "6", true);
	addEntry(0xB3, "LAX", "($44),Y" , "Indirect,Y" , 2, "5+", true);

	addEntry(0x87, "SAX", "$44"     , "Zero Page"  , 2, "3", true);
	addEntry(0x97, "SAX", "$44,X"   , "Zero Page,Y", 2, "4", true);
	addEntry(0x8F, "SAX", "$4400"   , "Absolute"   , 3, "4", true);
	addEntry(0x83, "SAX", "($44,X)" , "Indirect,X" , 2, "6", true);

	addEntry(0xEB, "SBC", "#$44"    , "Immediate"  , 2, "2", true);
}

size_t CPU6502Disassembler::disassemble(uint8_t opCode, uint8_t arg0, uint8_t arg1, gsl::span<char> dst) const
{
	const auto& e = entries[opCode];

	// Op codes
	const char unofficial = e.unofficial ? '*' : ' ';
	if (e.nBytes == 1) {
		std::snprintf(dst.data(), dst.size(), "%02hhX       %c%s", opCode, unofficial, e.mnemonic);
	} else if (e.nBytes == 2) {
		std::snprintf(dst.data(), dst.size(), "%02hhX %02hhX    %c%s", opCode, arg0, unofficial, e.mnemonic);
	} else if (e.nBytes == 3) {
		std::snprintf(dst.data(), dst.size(), "%02hhX %02hhX %02hhX %c%s", opCode, arg0, arg1, unofficial, e.mnemonic);
	}

	return e.nBytes;
}

void CPU6502Disassembler::addEntry(uint8_t opCode, const char* mnemonic, const char* parameter, const char* mode, uint8_t nBytes, const char* nCycles, bool unofficial)
{
	auto& e = entries[opCode];
	std::strncpy(e.mnemonic, mnemonic, 4);
	std::strncpy(e.parameter, parameter, 8);
	std::strncpy(e.mode, mode, 12);
	e.nBytes = nBytes;
	e.nCycles = nCycles[0] - '0';
	e.addCycleIfPage = nCycles[1] == '+';
	e.addCycleIfPage = nCycles[1] == '+' && nCycles[2] == '+';
	e.unofficial = unofficial;
};
