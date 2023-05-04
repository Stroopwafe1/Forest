#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <filesystem>

#include "Tokeniser.hpp"
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

	//std::cout << code << std::endl;

	//std::cout << "============================================" << std::endl;

	std::vector<Token> tokens = forest::parser::Tokeniser::parse(code, filePath);

	/*for (const Token& token : tokens) {
		token.debugPrint();
	}*/

	//std::cout << "============================================" << std::endl;

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
	uint32_t labelCount = 0;
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
	if (p.requires_libs) {
		outfile << "; Print function provided by Peter Cordes @ https://stackoverflow.com/a/46301894" << std::endl;
		outfile << "global print_uint32" << std::endl;
		outfile << "print_uint32:" << std::endl;
		outfile << "\tmov eax, edi" << std::endl;
		outfile << "\tmov ecx, 0xa" << std::endl;
		outfile << "\tpush 0x0" << std::endl;
		outfile << "\tmov rsi, rsp" << std::endl;
		outfile << "\tsub rsp, 16" << std::endl;
		outfile << std::endl << "toascii_digit:" << std::endl;
		outfile << "\txor edx, edx" << std::endl;
		outfile << "\tdiv ecx" << std::endl;
		outfile << "\tadd edx, '0'" << std::endl;
		outfile << "\tdec rsi" << std::endl;
		outfile << "\tmov [rsi], dl" << std::endl;
		outfile << "\ttest eax, eax" << std::endl;
		outfile << "\tjnz toascii_digit" << std::endl;
		outfile << "\tmov eax, 1" << std::endl;
		outfile << "\tmov edi, 1" << std::endl;
		outfile << "\tlea edx, [rsp+16 + 1]" << std::endl;
		outfile << "\tsub edx, esi" << std::endl;
		outfile << "\tsyscall" << std::endl;
		outfile << "\tadd rsp, 24" << std::endl;
		outfile << "\tret" << std::endl;

		outfile << "global print_uint32_newline" << std::endl;
		outfile << "print_uint32_newline:" << std::endl;
		outfile << "\tmov eax, edi" << std::endl;
		outfile << "\tmov ecx, 0xa" << std::endl;
		outfile << "\tpush rcx" << std::endl;
		outfile << "\tmov rsi, rsp" << std::endl;
		outfile << "\tsub rsp, 16" << std::endl;
		outfile << "\tjmp toascii_digit" << std::endl;
	}
	outfile << std::endl << "_start:" << std::endl;
	// TODO: Make this recursive
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
		} else if ((*it).mType == Statement_Type::LOOP) {
			if (!(*it).loopStatement.has_value()) continue;
			LoopStatement ls = (*it).loopStatement.value();
			outfile << "; =============== LOOP ===============" << std::endl;
			Builtin_Type t = ls.mIterator.value().mType.builtinType;
			std::string op = ls.mRange.value().mMinimum < ls.mRange.value().mMaximum ? "inc " : "dec ";
			std::string r1;
			std::string r2;

			if (t == I8 || t == UI8) {
				r1 = "r12b";
				r2 = "r13b";
			} else if (t == I16 || t == UI16) {
				r1 = "r12w";
				r2 = "r13w";
			} else if (t == I32 || t == UI32) {
				r1 = "r12d";
				r2 = "r13d";
			} else if (t == I64 || t == UI64) {
				r1 = "r12";
				r2 = "r13";
			}

			outfile << "\tmov " << r1 << ", " << ls.mRange.value().mMinimum << std::endl;
			outfile << "\tmov " << r2 << ", " << ls.mRange.value().mMaximum << std::endl;
			outfile << "label" << ++labelCount << ":" << std::endl;
			// Code inside of the loop here
			for (std::vector<Statement>::iterator lsit = ls.mBody.statements.begin(); lsit != ls.mBody.statements.end(); lsit++) {
				if (!(*lsit).funcCall.has_value()) continue;
				FuncCallStatement fc = (*lsit).funcCall.value();
				if (fc.arg.mType == TokenType::LITERAL) {
					outfile << "; =============== FUNC CALL ===============" << std::endl;
					outfile << "\tmov rax, " << (fc.mFunctionType == StdLib_Function_Type::READ || fc.mFunctionType == StdLib_Function_Type::READLN ? 0 : 1) << std::endl;
					outfile << "\tmov rdi, " << (fc.mClassType == StdLib_Class_Type::STDIN ? 0 : 1) << std::endl;
					outfile << "\tmov rsi, " << (*lsit).content << std::endl;
					outfile << "\tmov rdx, " << p.findLiteral((*lsit).content).value().mSize << std::endl;
					outfile << "\tsyscall" << std::endl;
					outfile << "; =============== END FUNC CALL ===============" << std::endl;
				} else if (fc.arg.mType == TokenType::IDENTIFIER) {
					outfile << "; =============== FUNC CALL ===============" << std::endl;
					outfile << "\tmovzx edi, " << r1 << std::endl;
					if (fc.mFunctionType == StdLib_Function_Type::WRITE) {
						outfile << "\tcall print_uint32" << std::endl;
					} else if (fc.mFunctionType == StdLib_Function_Type::WRITELN) {
						outfile << "\tcall print_uint32_newline" << std::endl;
					}
					outfile << "; =============== END FUNC CALL ===============" << std::endl;
				}
			}

			outfile << "\t" << op << r1 << std::endl;
			outfile << "\tcmp " << r1 << ", " << r2 << std::endl;
			outfile << "\tje not_label" << labelCount << std::endl;
			outfile << "\tjmp label" << labelCount << std::endl;
			outfile << "not_label" << labelCount << ":" << std::endl;
			outfile << "; =============== END LOOP ===============" << std::endl;
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
	//std::cout << linker.str().c_str() << std::endl;
	std::system(linker.str().c_str());

	std::stringstream run_command;
	run_command << "./build/" << fileName.stem().string();
	std::system(run_command.str().c_str());

	return 0;
}
