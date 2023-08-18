#ifndef FOREST_X86_64LINUXYASMCOMPILER_HPP
#define FOREST_X86_64LINUXYASMCOMPILER_HPP

#include <filesystem>
#include "Parser.hpp"
#include <map>

using namespace forest::parser;
namespace fs = std::filesystem;

struct SymbolInfo {
	std::string reg {};
	int offset {};
	Type type {};
	int size {};

	std::string location() const {
		std::stringstream ss;
		if (reg == "rbp") {
			ss << "[" << reg;
			if (offset > 0)
				ss << "+" << offset;
			else if (offset < 0)
				ss << offset;
			ss << "]";
		} else
			ss << reg;
		return ss.str();
	}
};

struct ExpressionPrinted {
	bool printed = false;
	bool sign = false;
	int size {};
};

class X86_64LinuxYasmCompiler {
public:
	void compile(fs::path& fileName, const Programme& p, const Function& main);

private:
	uint32_t labelCount = 0;
	uint32_t ifCount = 0;
	std::map<std::string, SymbolInfo> symbolTable;
	void printBody(std::ofstream& outfile, const Programme& p, const Block& block, const std::string& labelName, int* offset);
	void printLibs(std::ofstream& outfile);
	void printFunctionCall(std::ofstream& outfile, const Programme& p, const FuncCallStatement& fc);
	/**
	 * Will print the expression. The resulting value will be in the a register (rax, eax, ax, al)
	 */
	ExpressionPrinted printExpression(std::ofstream& outfile, const Programme& p, const Expression* expression, uint8_t nodeType);
	void printConditionalMove(std::ofstream& outfile, int leftSize, int rightSize, const char* instruction);
	int addToSymbols(int* offset, const Variable& variable, const std::string& reg = "rbp-");
	std::stringstream moveToRegister(const std::string& reg, const SymbolInfo& symbol);
	const char* getRegister(const std::string& reg, int size);
	int getSizeFromNumber(const std::string& text);
	int getEvenSize(int size1, int size2);
	const char* getMoveAction(int regSize, int valSize, bool isSigned);
	int getSizeFromByteSize(size_t byteSize);
};


#endif //FOREST_X86_64LINUXYASMCOMPILER_HPP
