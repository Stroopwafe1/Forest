#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

#include "Tokeniser.h"

using namespace forest::parser;

int main(int argc, char** argv) {
	const char* fileName = argc == 1 ? "../Examples/hello-world.tree" : argv[1];

	if (argc >= 2) {
		// Run with arguments
		std::cout << "Using file: " << fileName << std::endl;
	}

	std::ifstream file;
	file.open(fileName);
	std::stringstream buffer;
	buffer << file.rdbuf();

	std::string code = buffer.str();

	std::cout << code << std::endl;

	std::cout << "============================================" << std::endl;

	std::vector<Token> tokens = forest::parser::Tokeniser::parse(code, fileName);

	for (const Token& token : tokens) {
		token.debugPrint();
	}

	file.close();

	return 0;
}