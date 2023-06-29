#include <sstream>
#include <fstream>
#include <map>
#include "X86_64LinuxYasmCompiler.hpp"

std::string X86_64LinuxYasmCompiler::addToSymbols(size_t* offset, const Variable& variable) {
	std::string result;
	switch (variable.mType.builtinType) {
		case Builtin_Type::UNDEFINED:
		case Builtin_Type::VOID:
			break;
		case Builtin_Type::UI8:
		case Builtin_Type::I8:
		case Builtin_Type::F8:
		case Builtin_Type::CHAR:
			(*offset) += 1;
			result = "byte";
			break;
		case Builtin_Type::UI16:
		case Builtin_Type::I16:
		case Builtin_Type::F16:
			(*offset) += 2;
			result = "word";
			break;
		case Builtin_Type::UI32:
		case Builtin_Type::I32:
		case Builtin_Type::F32:
		case Builtin_Type::BOOL:
		case Builtin_Type::REF:
		case Builtin_Type::ARRAY:
			(*offset) += 4;
			result = "dword";
			break;
		case Builtin_Type::UI64:
		case Builtin_Type::I64:
		case Builtin_Type::F64:
			(*offset) += 8;
			result = "qword";
			break;
	}
	symbolTable.insert(std::make_pair(variable.mName, SymbolInfo {*offset, variable.mType, result}));
	return result;
}

void X86_64LinuxYasmCompiler::compile(fs::path& fileName, const Programme& p, const Function& main) {
	fs::create_directory("./build");
	fs::path outPath = "./build";
	outPath /= fileName.concat(".asm");
	std::ofstream outfile;
	outfile.open(outPath);

	labelCount = 0;
	outfile << "section .data" << std::endl;
	for (const auto& literal : p.literals) {
		outfile << "\t" << literal.mAlias << ": db \"";
		if (literal.mContent[literal.mContent.size() - 1] == '\n') {
			// Replace \n in string with ',0xA' put after the quote
			outfile << literal.mContent.substr(0, literal.mContent.size() - 1) << "\",0xA,0" << std::endl;
		} else {
			// Just write the string normally
			outfile << literal.mContent << "\"" << ",0" << std::endl;
		}
	}
	outfile << std::endl;
	outfile << "section .text" << std::endl;

	for (const auto& funcCall : p.externalFunctions) {
		outfile << "\textern " << funcCall.mFunctionName << std::endl;
	}

	outfile << "\tglobal _start" << std::endl;
	if (p.requires_libs) {
		printLibs(outfile);
	}

	// Loop over functions
	// Start with prologue 'push rbp', 'mov rbp, rsp'
	// For every variable, keep track of the offset
	// End with epilogue 'pop rbp'

	for (const auto& function : p.functions) {
		if (function.mName == "main") {
			outfile << std::endl << "_start:" << std::endl;
		} else {
			outfile << std::endl << function.mName << std::endl;
		}
		outfile << "; =============== PROLOGUE ===============" << std::endl;
		outfile << "\tpush rbp" << std::endl;
		outfile << "\tmov rbp, rsp" << std::endl;
		outfile << "; =============== END PROLOGUE ===============" << std::endl;


		// Construct symbol table, keeping track of scope
		// offset from stack, type
		std::map<std::string, SymbolInfo> symbolTable;
		size_t offset = 0;

		// TODO: Have function that compiles expressions

		printBody(outfile, p, function.mBody, function.mName, &offset);

		if (function.mName != "main") {
			outfile << "; =============== EPILOGUE ===============" << std::endl;
			outfile << "\tpop rbp" << std::endl;
			outfile << "; =============== END EPILOGUE ===============" << std::endl;
		}
	}

	outfile.close();

	bool debug = true;

	std::stringstream assembler;
	assembler << "yasm -f elf64";
	if (debug)
		assembler << " -g dwarf2";
	assembler << " -o ./build/" <<  fileName.stem().string() << ".o ./build/" << fileName.stem().string() << ".asm";
	//std::cout << assembler.str().c_str() << std::endl;
	std::system(assembler.str().c_str());

	std::stringstream linker;
	linker << "ld";
	if (!debug)
		linker << " -s -z noseparate-code ";
	else
		linker << " -g";
	linker << " -o ./build/" << fileName.stem().string() << " --dynamic-linker=/lib64/ld-linux-x86-64.so.2 ./build/" << fileName.stem().string() << ".o";
	for (auto& dependency : p.libDependencies) {
		linker << " -l" << dependency;
	}
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

void X86_64LinuxYasmCompiler::printFunctionCall(std::ofstream& outfile, const Programme& p, const FuncCallStatement& fc) {
	// TODO: We assume only one argument here
	// TODO: We assume entire expression tree is collapsed
	Expression* arg = fc.mArgs[0];
	if (arg->mValue.mType == TokenType::LITERAL && arg->mValue.mSubType == TokenSubType::STRING_LITERAL) {
		Literal l = p.findLiteralByContent(arg->mValue.mText).value();
		outfile << "; =============== FUNC CALL + STRING ===============" << std::endl;
		outfile << "\tmov rax, " << (fc.mFunctionName == "read" || fc.mFunctionName == "readln" ? 0 : 1) << std::endl;
		outfile << "\tmov rdi, " << (fc.mClassName == "stdin" ? 0 : 1) << std::endl;
		outfile << "\tmov rsi, " << l.mAlias << std::endl;
		outfile << "\tmov rdx, " << l.mSize << std::endl;
		outfile << "\tsyscall" << std::endl;
		outfile << "; =============== END FUNC CALL + STRING ===============" << std::endl;

	} else if (arg->mValue.mType == TokenType::LITERAL && arg->mValue.mSubType == TokenSubType::INTEGER_LITERAL) {
		outfile << "; =============== FUNC CALL + INT ===============" << std::endl;
		outfile << "\tmov edi, " << arg->mValue.mText << std::endl;
		if (fc.mFunctionName == "write") {
			outfile << "\tcall print_uint32" << std::endl;
		} else if (fc.mFunctionName == "writeln") {
			outfile << "\tcall print_uint32_newline" << std::endl;
		}
		outfile << "; =============== END FUNC CALL + INT ===============" << std::endl;

	} else if (arg->mValue.mType == TokenType::IDENTIFIER) {
		SymbolInfo& var = symbolTable[arg->mValue.mText];
		outfile << "; =============== FUNC CALL + VARIABLE ===============" << std::endl;
		outfile << "\tmovzx edi, " << var.size << " [rbp+" << var.offset << "]" << std::endl;
		if (fc.mFunctionName == "write") {
			outfile << "\tcall print_uint32" << std::endl;
		} else if (fc.mFunctionName == "writeln") {
			outfile << "\tcall print_uint32_newline" << std::endl;
		}
		outfile << "; =============== END FUNC CALL + VARIABLE ===============" << std::endl;

	} else if (arg->mValue.mType == TokenType::OPERATOR && arg->mValue.mText == "[") { // Array indexing
		// TODO: We can only index CLI arguments now, extend to generic solution for all arrays
		if (arg->mLeft->mValue.mText != "argv") throw std::runtime_error("Unexpected array index name");
		outfile << "\tmov rdi, qword [rsp+8+" << arg->mRight->mValue.mText << "*8] ; Move the CLI arg into rdi" << std::endl;
		outfile << "\tcall printString" << std::endl;
		if (fc.mFunctionName == "writeln") {
			outfile << "\tmov rax, 1" << std::endl;
			outfile << "\tmov rdi, 1" << std::endl;
			outfile << "\tmov rsi, 0xA" << std::endl;
			outfile << "\tmov rdx, 1" << std::endl;
			outfile << "\tsyscall" << std::endl;
		}
	}
}

void X86_64LinuxYasmCompiler::printBody(std::ofstream& outfile, const Programme& p, const Block& block, const std::string& labelName, size_t* offset) {
	const char callingConvention[6][4] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
	std::map<std::string, SymbolInfo> localSymbols;
	size_t localOffset = 0;

	for (const auto& statement : block.statements) {
		switch (statement.mType) {
			case Statement_Type::RETURN_CALL:
				if (labelName == "main") {
					outfile << "\tmov rax, 60" << std::endl;
					outfile << "\tmov rdi, " << statement.mContent->mValue.mText << std::endl;
					outfile << "\tsyscall" << std::endl;
				} else {
					outfile << "\tmov rax, " << statement.mContent->mValue.mText << std::endl;
					outfile << "\tret" << std::endl;
				}
				break;
			case Statement_Type::VAR_DECLARATION:
				addToSymbols(offset, statement.variable.value());
				addToSymbols(&localOffset, statement.variable.value());
				break;
			case Statement_Type::VAR_ASSIGNMENT: {
				addToSymbols(&localOffset, statement.variable.value());
				outfile << "\tmov " << addToSymbols(offset, statement.variable.value()) << " [rbp+"
						<< symbolTable[statement.variable.value().mName].offset << "], " << statement.mContent->mValue.mText
						<< std::endl;
				break;
			}
			case Statement_Type::LOOP: {
				if (!statement.loopStatement.has_value()) continue;
				LoopStatement ls = statement.loopStatement.value();
				if (ls.mIterator.has_value()) {
					std::string size = addToSymbols(offset, ls.mIterator.value());
					addToSymbols(&localOffset, ls.mIterator.value());
					std::string op = ls.mRange.value().mMinimum < ls.mRange.value().mMaximum ? "inc " : "dec ";
					std::string label = "label";
					label = label.append(std::to_string(++labelCount));
					outfile << "\tmov " << size << " [rbp+" << symbolTable[ls.mIterator.value().mName].offset << "], " << ls.mRange.value().mMinimum->mValue.mText << std::endl;
					outfile << label << ":" << std::endl;
					outfile << "\tcmp " << size << " [rbp+" << symbolTable[ls.mIterator.value().mName].offset << "], " << ls.mRange.value().mMaximum->mValue.mText << std::endl;
					outfile << "\tjne inside_label" << labelCount << std::endl;
					outfile << "\tjmp not_label" << labelCount << std::endl;
					outfile << "inside_label" << labelCount << ":" << std::endl;
					printBody(outfile, p, ls.mBody, label, offset);
					outfile << "\tmovzx eax, " << size << " [rbp+" << symbolTable[ls.mIterator.value().mName].offset << "]" << std::endl;
					outfile << "\t" << op << "eax" << std::endl;
					outfile << "\tmov " << size << " [rbp+" << symbolTable[ls.mIterator.value().mName].offset << "], al" << std::endl;
					outfile << "\tjmp label" << labelCount << std::endl;
					outfile << "not_label" << labelCount << ":" << std::endl;
				} else {
					if (statement.mContent == nullptr) {
						// We have "loop { ... }"
						std::string label = "label";
						label = label.append(std::to_string(++labelCount));
						outfile << label << ":" << std::endl;
						printBody(outfile, p, ls.mBody, label, offset);
						outfile << "\tjmp label" << labelCount << std::endl;
						outfile << "not_label" << labelCount << ":" << std::endl; // Used for break statements
					} else {
						// we have "until condition { ... }"
					}
				}
				break;
			}
			case Statement_Type::FUNC_CALL: {
				FuncCallStatement fc = statement.funcCall.value();
				// If function is stdlib call, need to expand this into something better when stdlib expands
				if (fc.mClassName == "stdout" || fc.mClassName == "stdin") {
					printFunctionCall(outfile, p, fc);
					break;
				}
				for (int i = statement.funcCall.value().mArgs.size() - 1; i >= 0; i--) {
					std::string value;
					Expression* expr = statement.funcCall.value().mArgs[i];
					if (expr->mValue.mSubType == forest::parser::TokenSubType::STRING_LITERAL) {
						value = p.findLiteralByContent(expr->mValue.mText)->mAlias;
					} else {
						value = expr->mValue.mText;
					}
					if (i <= 6)
						outfile << "\tmov " << callingConvention[i] << ", "	<< value << std::endl;
					else
						outfile << "\tpush " << value << std::endl;
				}
				outfile << "\tmov rax, 0" << std::endl;
				outfile << "\tcall " << statement.funcCall.value().mFunctionName << std::endl;
				break;
			}
			case Statement_Type::NOTHING:
				outfile << "\tnop" << std::endl;
				break;
			case Statement_Type::IF:
				break;
			case Statement_Type::ARRAY_INDEX:
				break;
		}
	}
	symbolTable.erase(localSymbols.cbegin(), localSymbols.cend());
}

const char* X86_64LinuxYasmCompiler::getRegister(const std::string& size, const std::string& reg) {
	return "";
}

void X86_64LinuxYasmCompiler::printExpression(std::ofstream& outfile, const Programme& p, const Expression* expression) {
	if (expression == nullptr) return;
	if (expression->mLeft == nullptr && expression->mRight == nullptr) {
		return;
	}
	if (expression->mValue.mType != TokenType::OPERATOR) {
		return;
	}
	if (expression->mLeft->mValue.mSubType == TokenSubType::STRING_LITERAL || expression->mRight->mValue.mSubType == TokenSubType::STRING_LITERAL)
		return;
	printExpression(outfile, p, expression->mLeft);
	printExpression(outfile, p, expression->mRight);

	// Here the fun starts
	// TODO: If left had nodes, then eax would already have a value. So fix this
	if (expression->mLeft->mValue.mType == TokenType::IDENTIFIER) {
		SymbolInfo& left = symbolTable[expression->mLeft->mValue.mText];
		outfile << "\tmovzx eax, " << left.size << " [rbp+" << left.offset << "]" << std::endl;
	} else if (expression->mLeft->mValue.mSubType == TokenSubType::INTEGER_LITERAL) {
		outfile << "\tmov eax, " << expression->mLeft->mValue.mText << std::endl;
	}
}
