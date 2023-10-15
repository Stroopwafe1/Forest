#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <filesystem>

#include "Tokeniser.hpp"
#include "Parser.hpp"
#include "ConfigParser.hpp"
#include "X86_64LinuxYasmCompiler.hpp"

using namespace forest::parser;
namespace fs = std::filesystem;

int main(int argc, char** argv) {
	// If the path is a directory, look for the config file
	fs::path filePath = argc == 1 || argc > 2 ? "../Examples/rule110.tree" : argv[1];
	bool useProject = false;
	if (fs::is_directory(filePath)) {
		// We are in a directory, parse config file
		for (const auto& entry : fs::directory_iterator(filePath)) {
			if (entry.path().filename() == "frstconfig") {
				filePath = entry.path();
				useProject = true;
				std::cout << "Direntry: " << entry.path() << std::endl;
			}
		}
		if (!useProject) {
			std::cout << "Calling Forest compile on a directory. Expected a file called `frstconfig` but found nothing";
			return -1;
		}
	}

	if (argc >= 2) {
		// Run with arguments
		std::cout << "Using file: " << filePath << std::endl;
	}

	std::ifstream file;
	file.open(filePath);
	std::stringstream buffer;
	buffer << file.rdbuf();
	file.close();

	std::string code = buffer.str();

	std::vector<Token> tokens = forest::parser::Tokeniser::parse(code, filePath);

	if (useProject) {
		// Relative to filepath, read entrypoint
		CompileContext ctx(tokens);
		tokens.clear();
		std::stringstream().swap(buffer);
		filePath = filePath.replace_filename(ctx.m_Configuration.m_Entrypoint);
		file.open(filePath);
		buffer << file.rdbuf();
		file.close();
		tokens = Tokeniser::parse(buffer.str(), filePath);
	}

	Parser parser;
	Programme p = parser.parse(tokens);
	bool found_main = false;

	for (auto& function : p.functions) {
		if (function.mName == "main") {
			found_main = true;
			break;
		}
	}
	if (!found_main) {
		std::cerr << "Expected a function called 'main' to be in file: " << filePath << std::endl;
		return 1;
	}

	X86_64LinuxYasmCompiler compiler;
	compiler.compile(filePath, p);

	return 0;
}
