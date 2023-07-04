#ifndef FOREST_X86_64LINUXYASMCOMPILER_HPP
#define FOREST_X86_64LINUXYASMCOMPILER_HPP

#include <filesystem>
#include "Parser.hpp"
#include <map>

using namespace forest::parser;
namespace fs = std::filesystem;

struct SymbolInfo {
	size_t offset = 0;
	Type type {};
	std::string size;
};

class X86_64LinuxYasmCompiler {
public:
	void compile(fs::path& fileName, const Programme& p, const Function& main);

private:
	uint32_t labelCount = 0;
	std::map<std::string, SymbolInfo> symbolTable;
	void printBody(std::ofstream& outfile, const Programme& p, const Block& block, const std::string& labelName, size_t* offset);
	void printLibs(std::ofstream& outfile);
	void printFunctionCall(std::ofstream& outfile, const Programme& p, const FuncCallStatement& fc);
	/**
	 * Will print the expression. The resulting value will be in the a register (rax, eax, ax, al)
	 */
	bool printExpression(std::ofstream& outfile, const Programme& p, const Expression* expression, uint8_t nodeType);
	std::string addToSymbols(size_t* offset, const Variable& variable);
	std::stringstream moveToRegister(const std::string& reg, const SymbolInfo& symbol);
};


#endif //FOREST_X86_64LINUXYASMCOMPILER_HPP
