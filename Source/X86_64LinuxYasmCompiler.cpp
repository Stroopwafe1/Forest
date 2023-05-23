#include <sstream>
#include <fstream>
#include "X86_64LinuxYasmCompiler.hpp"

void X86_64LinuxYasmCompiler::compile(fs::path& fileName, const Programme& p, const Function& main) {
	fs::create_directory("./build");
	fs::path outPath = "./build";
	outPath /= fileName.concat(".asm");
	std::ofstream outfile;
	outfile.open(outPath);

	// Compile main. TODO: Also add other functions
	uint32_t labelCount = 0;
	outfile << "section .data" << std::endl;
	for (const auto& literal : p.literals) {
		outfile << "\t" << literal.mAlias << " db \"";
		if (literal.mContent[literal.mContent.size() - 1] == '\n') {
			// Replace \n in string with ',0xA' put after the quote
			outfile << literal.mContent.substr(0, literal.mContent.size() - 1) << "\",0xA" << std::endl;
		} else {
			// Just write the string normally
			outfile << literal.mContent << "\"" << std::endl;
		}
	}
	outfile << std::endl;
	outfile << "section .text" << std::endl;
	outfile << "\tglobal _start" << std::endl;
	if (p.requires_libs) {
		printLibs(outfile);
	}
	outfile << std::endl << "_start:" << std::endl;
	// TODO: Make this recursive
	for (const auto& statement : main.mBody.statements) {
		if (statement.mType == Statement_Type::RETURN_CALL) {
			outfile << "; =============== RETURN ===============" << std::endl;
			outfile << "\tmov rax, 60" << std::endl;
			outfile << "\tmov rdi, " << statement.mContent->mValue.mText << std::endl;
			outfile << "\tsyscall" << std::endl;
			outfile << "; =============== END RETURN ===============" << std::endl;
		} else if (statement.mType == Statement_Type::FUNC_CALL) {
			if (!statement.funcCall.has_value()) continue;
			FuncCallStatement fc = statement.funcCall.value();
			printFunctionCall(outfile, p, fc, "");

		} else if (statement.mType == Statement_Type::LOOP) {
			if (!statement.loopStatement.has_value()) continue;
			LoopStatement ls = statement.loopStatement.value();
			outfile << "; =============== LOOP ===============" << std::endl;
			Builtin_Type t = ls.mIterator.value().mType.builtinType;
			std::string op = ls.mRange.value().mMinimum < ls.mRange.value().mMaximum ? "inc " : "dec ";
			std::string r1;
			std::string r2;

			if (t == Builtin_Type::I8 || t == Builtin_Type::UI8) {
				r1 = "r12b";
				r2 = "r13b";
			} else if (t == Builtin_Type::I16 || t == Builtin_Type::UI16) {
				r1 = "r12w";
				r2 = "r13w";
			} else if (t == Builtin_Type::I32 || t == Builtin_Type::UI32) {
				r1 = "r12d";
				r2 = "r13d";
			} else if (t == Builtin_Type::I64 || t == Builtin_Type::UI64) {
				r1 = "r12";
				r2 = "r13";
			}

			// IDEA: If users start to loop loops, we can have the innermost loop use the registers, and the outer loops pushing their values onto the stack and then retrieve into the registers

			outfile << "\tmov " << r1 << ", " << ls.mRange.value().mMinimum << std::endl;
			outfile << "\tmov " << r2 << ", " << ls.mRange.value().mMaximum << std::endl;
			outfile << "label" << ++labelCount << ":" << std::endl;
			// Code inside the loop here
			for (auto& lsit : ls.mBody.statements) {
				if (!lsit.funcCall.has_value()) continue;
				FuncCallStatement fc = lsit.funcCall.value();
				// Again, we assume only one argument, and it being fully collapsed
				printFunctionCall(outfile, p, fc, r1);
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
}

void X86_64LinuxYasmCompiler::printLibs(std::ofstream& outfile) {
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

	outfile << "global printString" << std::endl;
	outfile << "printString:" << std::endl;
	outfile << "\tpush rbp" << std::endl;
	outfile << "\tmov rbp, rsp" << std::endl;
	outfile << "\tpush rbx" << std::endl;
	outfile << "\tmov rbx, rdi ; Count characters in string" << std::endl;
	outfile << "\tmov rdx, 0" << std::endl;
	outfile << "strCountLoop:" << std::endl;
	outfile << "\tcmp byte [rbx], 0x0" << std::endl;
	outfile << "\tje strCountDone" << std::endl;
	outfile << "\tinc rdx" << std::endl;
	outfile << "\tinc rbx" << std::endl;
	outfile << "\tjmp strCountLoop" << std::endl;
	outfile << "strCountDone:" << std::endl;
	outfile << "\tcmp rdx, 0" << std::endl;
	outfile << "\tje prtDone" << std::endl;
	outfile << "\t; Actually call the syscall now" << std::endl;
	outfile << "\tmov rax, 1" << std::endl;
	outfile << "\tmov rsi, rdi" << std::endl;
	outfile << "\tmov rdi, 1" << std::endl;
	outfile << "\tsyscall" << std::endl;
	outfile << "prtDone:" << std::endl;
	outfile << "\tpop rbx" << std::endl;
	outfile << "\tpop rbp" << std::endl;
	outfile << "\tret" << std::endl;
}

void X86_64LinuxYasmCompiler::printFunctionCall(std::ofstream& outfile, const Programme& p, const FuncCallStatement& fc, const std::string& varRegister) {
	// TODO: We assume only one argument here
	// TODO: We assume entire expression tree is collapsed
	Expression* arg = fc.mArgs[0];
	if (arg->mValue.mType == TokenType::LITERAL && arg->mValue.mSubType == TokenSubType::STRING_LITERAL) {
		Literal l = p.findLiteralByContent(arg->mValue.mText).value();
		outfile << "; =============== FUNC CALL + STRING ===============" << std::endl;
		outfile << "\tmov rax, " << (fc.mFunctionType == StdLib_Function_Type::READ || fc.mFunctionType == StdLib_Function_Type::READLN ? 0 : 1) << std::endl;
		outfile << "\tmov rdi, " << (fc.mClassType == StdLib_Class_Type::STDIN ? 0 : 1) << std::endl;
		outfile << "\tmov rsi, " << l.mAlias << std::endl;
		outfile << "\tmov rdx, " << l.mSize << std::endl;
		outfile << "\tsyscall" << std::endl;
		outfile << "; =============== END FUNC CALL + STRING ===============" << std::endl;

	} else if (arg->mValue.mType == TokenType::LITERAL && arg->mValue.mSubType == TokenSubType::INTEGER_LITERAL) {
		outfile << "; =============== FUNC CALL + INT ===============" << std::endl;
		outfile << "\tmov edi, " << arg->mValue.mText << std::endl;
		if (fc.mFunctionType == StdLib_Function_Type::WRITE) {
			outfile << "\tcall print_uint32" << std::endl;
		} else if (fc.mFunctionType == StdLib_Function_Type::WRITELN) {
			outfile << "\tcall print_uint32_newline" << std::endl;
		}
		outfile << "; =============== END FUNC CALL + INT ===============" << std::endl;

	} else if (arg->mValue.mType == TokenType::IDENTIFIER) {
		outfile << "; =============== FUNC CALL + VARIABLE ===============" << std::endl;
		outfile << "\tmovzx edi, " << varRegister << std::endl;
		if (fc.mFunctionType == StdLib_Function_Type::WRITE) {
			outfile << "\tcall print_uint32" << std::endl;
		} else if (fc.mFunctionType == StdLib_Function_Type::WRITELN) {
			outfile << "\tcall print_uint32_newline" << std::endl;
		}
		outfile << "; =============== END FUNC CALL + VARIABLE ===============" << std::endl;

	} else if (arg->mValue.mType == TokenType::OPERATOR && arg->mValue.mText == "[") { // Array indexing
		// TODO: We can only index CLI arguments now, extend to generic solution for all arrays
		if (arg->mLeft->mValue.mText != "argv") throw std::runtime_error("Unexpected array index name");
		outfile << "\tmov rdi, qword [rsp+8+" << arg->mRight->mValue.mText << "*8] ; Move the CLI arg into rdi" << std::endl;
		outfile << "\tcall printString" << std::endl;
		if (fc.mFunctionType == StdLib_Function_Type::WRITELN) {
			outfile << "\tmov rax, 1" << std::endl;
			outfile << "\tmov rdi, 1" << std::endl;
			outfile << "\tmov rsi, 0xA" << std::endl;
			outfile << "\tmov rdx, 1" << std::endl;
			outfile << "\tsyscall" << std::endl;
		}
	}
}
