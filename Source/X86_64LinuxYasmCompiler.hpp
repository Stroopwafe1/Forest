#ifndef FOREST_X86_64LINUXYASMCOMPILER_HPP
#define FOREST_X86_64LINUXYASMCOMPILER_HPP

#include <filesystem>
#include "Parser.hpp"

using namespace forest::parser;
namespace fs = std::filesystem;

class X86_64LinuxYasmCompiler {
public:
	static void compile(fs::path& fileName, const Programme& p, const Function& main);
};


#endif //FOREST_X86_64LINUXYASMCOMPILER_HPP
