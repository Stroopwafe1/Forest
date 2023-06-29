#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <filesystem>

#include "Tokeniser.hpp"
#include "Parser.hpp"
#include "X86_64LinuxYasmCompiler.hpp"

using namespace forest::parser;
namespace fs = std::filesystem;

int main(int argc, char** argv) {
	//fs::path filePath = argc == 1 ? "../Examples/print-first-arg.tree" : argv[1];
	//fs::path filePath = argc == 1 ? "../Examples/seq100.tree" : argv[1];
	fs::path filePath = argc == 1 ? "../Examples/libc.tree" : argv[1];
	fs::path fileName = filePath.stem();

	if (argc >= 2) {
		// Run with arguments
		std::cout << "Using file: " << filePath << std::endl;
	}

	std::ifstream file;
	file.open(filePath);
	std::stringstream buffer;
	buffer << file.rdbuf();

	std::string code = buffer.str();

	std::vector<Token> tokens = forest::parser::Tokeniser::parse(code, filePath);

	/*for (const Token& token : tokens) {
		token.debugPrint();
	}*/

	Parser parser;
	Programme p = parser.parse(tokens);
	bool found_main = false;
	Function main;

	for (auto& function : p.functions) {
		if (function.mName == "main") {
			found_main = true;
			main = function;
		}
	}
	if (!found_main) {
		std::cerr << "Expected a function called 'main' to be in file: " << filePath << std::endl;
		return 1;
	}

	X86_64LinuxYasmCompiler compiler;
	compiler.compile(fileName, p, main);

	return 0;
}
