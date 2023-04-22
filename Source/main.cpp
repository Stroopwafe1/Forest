#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <filesystem>

#include "Tokeniser.h"
#include "Parser.hpp"

using namespace forest::parser;
namespace fs = std::filesystem;

int main(int argc, char** argv) {
	fs::path filePath = argc == 1 ? "../Examples/hello-world.tree" : argv[1];
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

	std::cout << code << std::endl;

	std::cout << "============================================" << std::endl;

	std::vector<Token> tokens = forest::parser::Tokeniser::parse(code, filePath);

	for (const Token& token : tokens) {
		token.debugPrint();
	}

	std::cout << "============================================" << std::endl;

	Parser parser;
	Programme p = parser.parse(tokens);
	bool found_main = false;
	Function main;

	for (std::vector<Function>::iterator it = p.functions.begin(); it != p.functions.end(); it++) {
		if ((*it).mName == "main") {
			found_main = true;
			main = *it;
		}
	}
	if (!found_main) {
		std::cerr << "Expected a function called 'main' to be in file: " << filePath << std::endl;
		return 1;
	}

	fs::create_directory("./build");
	fs::path outPath = "./build";
	outPath /= fileName.concat(".asm");
	std::ofstream outfile;
	outfile.open(outPath);

	// Compile main. TODO: Also add other functions
	outfile << "section .data" << std::endl;
	for (std::vector<Literal>::iterator it = p.literals.begin(); it != p.literals.end(); it++) {
		outfile << "\t" << (*it).mAlias << " db \"";
		if ((*it).mContent[(*it).mContent.size() - 1] == '\n') {
			// Replace \n in string with ',0xA' put after the quote
			outfile << (*it).mContent.substr(0, (*it).mContent.size() - 1) << "\",0xA" << std::endl;
		} else {
			// Just write the string normally
			outfile << (*it).mContent << "\"" << std::endl;
		}
	}
	outfile << std::endl;
	outfile << "section .text" << std::endl;
	outfile << "\tglobal _start" << std::endl;
	outfile << std::endl << "_start:" << std::endl;
	for (std::vector<Statement>::iterator it = main.mBody.statements.begin(); it != main.mBody.statements.end(); it++) {
		if ((*it).mType == Statement_Type::RETURN_CALL) {
			outfile << "; =============== RETURN ===============" << std::endl;
			outfile << "\tmov rax, 60" << std::endl;
			outfile << "\tmov rdi, " << (*it).content << std::endl;
			outfile << "\tsyscall" << std::endl;
			outfile << "; =============== END RETURN ===============" << std::endl;
		} else if ((*it).mType == Statement_Type::FUNC_CALL) {
			if (!(*it).funcCall.has_value()) continue;
			FuncCallStatement fc = (*it).funcCall.value();
			outfile << "; =============== FUNC CALL ===============" << std::endl;
			outfile << "\tmov rax, " << (fc.mFunctionType == StdLib_Function_Type::READ || fc.mFunctionType == StdLib_Function_Type::READLN ? 0 : 1) << std::endl;
			outfile << "\tmov rdi, " << (fc.mClassType == StdLib_Class_Type::STDIN ? 0 : 1) << std::endl;
			outfile << "\tmov rsi, " << (*it).content << std::endl;
			outfile << "\tmov rdx, " << p.findLiteral((*it).content).value().mSize << std::endl;
			outfile << "\tsyscall" << std::endl;
			outfile << "; =============== END FUNC CALL ===============" << std::endl;
		}
	}

	outfile.close();
	file.close();

	std::stringstream assembler;
	assembler << "yasm -f elf64 -o ./build/" <<  fileName.stem().string() << ".o ./build/" << fileName.stem().string() << ".asm";
	//std::cout << assembler.str().c_str() << std::endl;
	std::system(assembler.str().c_str());

	std::stringstream linker;
	linker << "ld -s -z noseparate-code -o ./build/" << fileName.stem().string() << " ./build/" << fileName.stem().string() << ".o";
	std::cout << linker.str().c_str() << std::endl;
	std::system(linker.str().c_str());

	std::stringstream run_command;
	run_command << "./build/" << fileName.stem().string();
	std::system(run_command.str().c_str());

	return 0;
}
