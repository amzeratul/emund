#include <vector>
#include <cstdint>
#include <gsl/span>

class CPU6502Disassembler {
public:
	CPU6502Disassembler();

	size_t disassemble(uint8_t opCode, uint8_t arg0, uint8_t arg1, gsl::span<char> dst) const;

private:
	class Entry {
	public:
		char mnemonic[4] = { 0 };
		char parameter[8] = { 0 };
		char mode[12] = { 0 };
		uint8_t nBytes = 1;
		uint8_t nCycles = 1;
		bool addCycleIfPage = false;
		bool addCycleIfTaken = false;
		bool unofficial = false;
	};

	std::vector<Entry> entries;
	
	void addEntry(uint8_t opCode, const char* mnemonic, const char* parameter, const char* mode, uint8_t nBytes, const char* nCycles, bool unofficial = false);
};
