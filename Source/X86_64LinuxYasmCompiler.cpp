#include <sstream>
#include <fstream>
#include <map>
#include <algorithm>
#include "X86_64LinuxYasmCompiler.hpp"

X86_64LinuxYasmCompiler::X86_64LinuxYasmCompiler() {
	syscallTable = {
	{"SYS_READ", 0},
	{"SYS_WRITE", 1},
	{"SYS_OPEN", 2},
	{"SYS_CLOSE", 3},
	{"SYS_STAT", 4},
	{"SYS_FSTAT", 5},
	{"SYS_LSTAT", 6},
	};
	// TODO: Fill this with more;
}


int X86_64LinuxYasmCompiler::addToSymbols(int* offset, const Variable& variable, const std::string& reg, bool isGlobal) {
	int result = getSizeFromType(variable.mType);

	if (!isGlobal) {
		std::string checkReg = reg.substr(0, 3);
		char op = reg.size() == 4 ? reg[3] : '-';
		if (checkReg == "rbp") {
			if (op == '-')
				(*offset) -= 1 << result;
			else
				(*offset) += 1 << result;
		}
		symbolTable.insert(std::make_pair(variable.mName, SymbolInfo {checkReg, *offset, variable.mType, result, isGlobal}));
	} else {
		symbolTable.insert(std::make_pair(variable.mName, SymbolInfo {variable.mName, 0, variable.mType, result, isGlobal}));
	}
	return result;
}

void X86_64LinuxYasmCompiler::compile(fs::path& filePath, const Programme& p, const CompileContext& ctx) {
	fs::path fileName = filePath.stem();
	std::string parentPath = filePath.parent_path().string();
	fs::path buildPath = filePath.parent_path() / "build";
	fs::create_directory(buildPath);
	fs::path outPath = buildPath;
	outPath /= fileName.concat(".asm");
	std::ofstream outfile;
	outfile.open(outPath);

	std::vector<Variable> constantVars;
	std::vector<Variable> initVars;
	std::vector<Variable> otherVars;

	for (const auto& kv : p.variables) {
		auto& c = ctx.getSymbolConvention(kv.first);
		if (c != ctx.conventionsEnd) {
			if (((*c).modifiers & Modifiers::CONSTANT) == Modifiers::CONSTANT) {
				constantVars.push_back(kv.second);
			} else if (!kv.second.mValues.empty()) {
				initVars.push_back(kv.second);
			} else {
				otherVars.push_back(kv.second);
			}
		}
	}

	labelCount = 0;
	ifCount = 0;
	if (!constantVars.empty()) {
		outfile << "section .rodata" << std::endl;
		for (const auto& constVar : constantVars) {
			addToSymbols(nullptr, constVar, constVar.mName, true);
			outfile << "\t" << constVar.mName << " " << getDefineBytes(constVar.mType.byteSize) << " ";
			for (int i = 0; i < constVar.mValues.size(); i++) {
				outfile << constVar.mValues[i]->mValue.mText;
				if (i != constVar.mValues.size() - 1)
					outfile << ", ";
			}
			outfile << std::endl;
		}
	}
	if (!initVars.empty() || !p.literals.empty()) {
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
		for (const auto& initVar : initVars) {
			addToSymbols(nullptr, initVar, initVar.mName, true);
			outfile << "\t" << initVar.mName << " " << getDefineBytes(initVar.mType.byteSize) << " ";
			for (int i = 0; i < initVar.mValues.size(); i++) {
				outfile << initVar.mValues[i]->mValue.mText;
				if (i != initVar.mValues.size() - 1)
					outfile << ", ";
			}
			outfile << std::endl;
		}
	}
	if (!otherVars.empty()) {
		outfile << "section .bss" << std::endl;
		for (const auto& otherVar : otherVars) {
			addToSymbols(nullptr, otherVar, otherVar.mName, true);
			if (otherVar.mType.builtinType != Builtin_Type::ARRAY)
				outfile << "\t" << otherVar.mName << " " << getReserveBytes(otherVar.mType.byteSize) << " " << otherVar.mValues.size() << std::endl;
			else
				outfile << "\t" << otherVar.mName << " " << getReserveBytes(otherVar.mType.subTypes[0].byteSize) << " " << otherVar.mType.byteSize /otherVar.mType.subTypes[0].byteSize << std::endl;
		}
	}
	outfile << std::endl;
	outfile << "section .text" << std::endl;

	std::vector<FuncCallStatement> external;
	for (const auto& funcCall : p.externalFunctions) {
		if (std::find(external.begin(), external.end(), funcCall) == external.end()) {
			outfile << "\textern ";
			if (!funcCall.mNamespace.empty())
				outfile << funcCall.mNamespace << "_";
			if (!funcCall.mClassName.empty())
				outfile << funcCall.mClassName << "_";
			outfile << funcCall.mFunctionName << std::endl;
			external.push_back(funcCall);
		}
	}

	if (p.requires_libs) {
		printLibs(outfile);
	}
	// Loop over functions
	// Start with prologue 'push rbp', 'mov rbp, rsp'
	// For every variable, keep track of the offset
	// End with epilogue 'pop rbp'

	const char callingConvention[6][4] = {"di", "si", "d", "c", "8", "9"};

	for (const auto& klass : p.classes) {
		currentClass = klass.first;
		for (const auto& function : klass.second.mFunctions) {
			const auto& convention = ctx.getSymbolConvention(function.mName);
			if (convention != ctx.conventionsEnd) {
				if (((*convention).modifiers & Modifiers::PUBLIC) == Modifiers::PUBLIC) {
					outfile << "global " << klass.first << "_" << function.mName << std::endl;
				}
			}
			outfile << klass.first << "_" << function.mName << ":" << std::endl;
			outfile << "; =============== PROLOGUE ===============" << std::endl;
			outfile << "\tpush rbp" << std::endl;
			outfile << "\tmov rbp, rsp" << std::endl;
			outfile << "; =============== END PROLOGUE ===============" << std::endl;

			int offset = -int(function.mBody.biggestAlloc);
			int argOffset = 0;

			std::vector<std::string> localSymbols;

			for (size_t i = 0; i < function.mArgs.size(); i++) {
				const auto& arg = function.mArgs[i];
				std::string reg = i < 6 ? getRegister(callingConvention[i], getSizeFromByteSize(arg.mType.byteSize)) : "rbp+";

				addToSymbols(&argOffset, Variable{arg.mType, arg.mName, {}}, reg);
				localSymbols.push_back(arg.mName);
			}

			int allocs = 0;
			printBody(outfile, p, function.mBody, function.mName, &offset, &allocs);

			for (const auto& symbolName : localSymbols) {
				symbolTable.erase(symbolName);
			}

			outfile << ".exit:" << std::endl;
			outfile << "; =============== EPILOGUE ===============" << std::endl;
			outfile << "\tpop rbp" << std::endl;
			outfile << "\tret" << std::endl;
			outfile << "; =============== END EPILOGUE ===============" << std::endl;
		}
	}
	for (const auto& function : p.functions) {
		if (function.mName == "main") {
			outfile << "\tglobal _start" << std::endl;
			outfile << std::endl << "_start:" << std::endl;
		} else {
			outfile << std::endl;
			const auto& convention = ctx.getSymbolConvention(function.mName);
			if (convention != ctx.conventionsEnd) {
				if (((*convention).modifiers & Modifiers::PUBLIC) == Modifiers::PUBLIC) {
					outfile << "global "  << function.mName << std::endl;
				}
			}
			outfile << function.mName << ":" << std::endl;
		}
		outfile << "; =============== PROLOGUE ===============" << std::endl;
		outfile << "\tpush rbp" << std::endl;
		outfile << "\tmov rbp, rsp" << std::endl;
		outfile << "; =============== END PROLOGUE ===============" << std::endl;


		// Construct symbol table, keeping track of scope
		// offset from stack, type
		int offset = -int(function.mBody.biggestAlloc);
		int argOffset = 0;

		std::vector<std::string> localSymbols;
		if (function.mName == "main") {
			const auto& argv = function.mArgs[0];
			argOffset = 16;
			addToSymbols(&argOffset, Variable {argv.mType.subTypes[0], argv.mName, {} }, "rsp");
			localSymbols.push_back(argv.mName);
		} else {
			for (size_t i = 0; i < function.mArgs.size(); i++) {
				const auto& arg = function.mArgs[i];
				std::string reg = i < 6 ? getRegister(callingConvention[i], getSizeFromByteSize(arg.mType.byteSize)) : "rbp+";

				addToSymbols(&argOffset, Variable{arg.mType, arg.mName, {}}, reg);
				localSymbols.push_back(arg.mName);
			}
		}

		int allocs = 0;
		printBody(outfile, p, function.mBody, function.mName, &offset, &allocs);

		for (const auto& symbolName : localSymbols) {
			symbolTable.erase(symbolName);
		}

		outfile << ".exit:" << std::endl;
		if (function.mName != "main") {
			outfile << "; =============== EPILOGUE ===============" << std::endl;
			outfile << "\tpop rbp" << std::endl;
			outfile << "\tret" << std::endl;
			outfile << "; =============== END EPILOGUE ===============" << std::endl;
		} else {
			outfile << "\tmov rax, 60" << std::endl;;
			outfile << "\tsyscall" << std::endl;
		}
	}

	outfile.close();

	std::stringstream assembler;
	assembler << "yasm -f elf64";
	if (ctx.m_Configuration.m_BuildType == BuildType::DEBUG)
		assembler << " -g dwarf2";
	assembler << " -o " << buildPath.string() << "/" << fileName.stem().string() << ".o " << buildPath.string() << "/" << fileName.stem().string() << ".asm";
	//std::cout << assembler.str().c_str() << std::endl;
	std::system(assembler.str().c_str());
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
	const char* sizes[] = {"byte", "word", "dword", "qword"};
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
		outfile << "\tmov rdi, " << arg->mValue.mText << std::endl;
		if (fc.mFunctionName == "write") {
			outfile << "\tcall print_uint32" << std::endl;
		} else if (fc.mFunctionName == "writeln") {
			outfile << "\tcall print_uint32_newline" << std::endl;
		}
		outfile << "; =============== END FUNC CALL + INT ===============" << std::endl;

	} else if (arg->mValue.mType == TokenType::IDENTIFIER) {
		SymbolInfo& var = symbolTable[arg->mValue.mText];
		outfile << "; =============== FUNC CALL + VARIABLE ===============" << std::endl;
		const char* moveAction = getMoveAction(3, var.size, false);
		outfile << "\t" << moveAction << " rdi, " << sizes[var.size] << " " << var.location() << std::endl;
		if (fc.mFunctionName == "write") {
			outfile << "\tcall print_uint32" << std::endl;
		} else if (fc.mFunctionName == "writeln") {
			outfile << "\tcall print_uint32_newline" << std::endl;
		}
		outfile << "; =============== END FUNC CALL + VARIABLE ===============" << std::endl;

	} else if (arg->mValue.mType == TokenType::OPERATOR && arg->mValue.mText == "[") { // Array indexing
		SymbolInfo& var = symbolTable[arg->mChildren[0]->mValue.mText];
		std::string offset = arg->mChildren[1]->mValue.mText;
		outfile << "\tmov rdi, " << sizes[var.size] << " [" << var.reg;
		if (var.offset < 0)
			outfile << offset << "*" << (1 << var.size) ;
		else if (var.offset > 0)
			outfile << "+" << var.offset << "+" << offset << "*" << (1 << var.size) ;
		outfile << "]" << std::endl;
		//outfile << "\tmov rdi, qword [rsp+8+" << offset << "*8] ; Move the CLI arg into rdi" << std::endl;
		outfile << "\tcall printString" << std::endl;
		if (fc.mFunctionName == "writeln") {
			outfile << "\tmov rax, 1" << std::endl;
			outfile << "\tmov rdi, 1" << std::endl;
			outfile << "\tmov rsi, 0xA" << std::endl;
			outfile << "\tmov rdx, 1" << std::endl;
			outfile << "\tsyscall" << std::endl;
		}
	} else if (arg->mValue.mType == TokenType::OPERATOR && arg->mValue.mText == ".") { // Struct indexing
		outfile << "; We don't support struct indexing yet" << std::endl;
	}
}

void X86_64LinuxYasmCompiler::printSyscall(std::ofstream& outfile, const std::string& syscall) {
	uint32_t call = syscallTable[syscall];
	if (call <= 6)
		outfile << "\tmov rax, " << call << std::endl;
	else {
		std::string error = "Unexpected syscall '$', please expand syscall table";
		error.replace(error.find_first_of('$'), 1, std::to_string(call));
		throw std::runtime_error(error);
	}
}


void X86_64LinuxYasmCompiler::printBody(std::ofstream& outfile, const Programme& p, const Block& block, const std::string& labelName, int* offset, int* allocs) {
	const char callingConvention[6][4] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
	const char* sizes[] = {"byte", "word", "dword", "qword"};
	std::vector<std::string> localSymbols;
	int localOffset = 0;
	if (block.stackMemory != 0) {
		outfile << "\tsub rsp, " << block.stackMemory + block.biggestAlloc << std::endl;
		(*allocs) += (block.stackMemory + block.biggestAlloc);
	}

	for (size_t i = 0; i < block.statements.size(); i++) {
		const auto& statement = block.statements[i];
		switch (statement.mType) {
			case Statement_Type::RETURN_CALL:
				printExpression(outfile, p, statement.mContent, 0);
				if (labelName == "main") {
					outfile << "\tmov rdi, rax" << std::endl;
				} else {
					if (*allocs > 0)
						outfile << "\tadd rsp, " << *allocs << std::endl;
					outfile << "\tjmp .exit" << std::endl;
				}

				break;
			case Statement_Type::VAR_DECLARATION:
				addToSymbols(offset, statement.variable.value());
				addToSymbols(&localOffset, statement.variable.value());
				localSymbols.push_back(statement.variable.value().mName);
				break;
			case Statement_Type::VAR_DECL_ASSIGN: {
				Variable v = statement.variable.value();
				if (v.mType.builtinType == Builtin_Type::ARRAY) {
					// Note: This subtraction is because we want to make the array initialise upwards towards the top of the stack
					// Reason for this is because register indexing is not allowed to go -rax, only +rax
					(*offset) -= int(v.mType.byteSize);
					localOffset -= int(v.mType.byteSize);
					addToSymbols(offset, v);
					addToSymbols(&localOffset, v);
					localSymbols.push_back(v.mName);

					SymbolInfo& arr = symbolTable[v.mName];
					int actualSize = getSizeFromByteSize(arr.type.subTypes[0].byteSize);

					for (int i = 0; i < v.mValues.size(); i++) {
						printExpression(outfile, p, v.mValues[i], 0);
						outfile << "\tmov " << sizes[actualSize] << " [" << arr.reg;
						if (arr.offset > 0)
							outfile << "+" << arr.offset - i * int(arr.type.byteSize / v.mValues.size());
						else if (arr.offset <= 0)
							outfile << "-" << -(arr.offset + i * int(arr.type.byteSize / v.mValues.size()));
						outfile << "], " << getRegister("a", actualSize) << std::endl;
					}
				} else if (v.mType.builtinType == Builtin_Type::STRUCT) {
					const Struct& s = p.structs.at(v.mType.name);
					(*offset) -= int(s.mSize);
					localOffset -= int(s.mSize);
					addToSymbols(offset, v);
					addToSymbols(&localOffset, v);
					localSymbols.push_back(v.mName);
					SymbolInfo& var = symbolTable[v.mName];

					for (int i = 0; i < s.mFields.size(); i++) {
						Expression* exp = v.mValues.at(i);
						if (exp == nullptr) continue;
						printExpression(outfile, p, exp, 0);
						int actualSize = getSizeFromByteSize(s.mFields[i].mType.byteSize); // TODO: This won't work for nested structs
						outfile << "\tmov " << sizes[actualSize] << " [" << var.reg;
						if (var.offset > 0)
							outfile << "+" << var.offset - s.mFields[i].mOffset;
						else
							outfile << "-" << -(var.offset + s.mFields[i].mOffset);
						outfile << "], " << getRegister("a", actualSize) << std::endl;
					}
				} else if (v.mType.builtinType == Builtin_Type::CLASS) {
					const Class& c = p.classes.at(v.mType.name);
					(*offset) -= int(c.mSize);
					localOffset -= int(c.mSize);
					addToSymbols(offset, v);
					addToSymbols(&localOffset, v);
					localSymbols.push_back(v.mName);
					SymbolInfo& var = symbolTable[v.mName];

					for (int i = 0; i < c.mFields.size(); i++) {
						Expression* exp = v.mValues.at(i);
						if (exp == nullptr) continue;
						printExpression(outfile, p, exp, 0);
						int actualSize = getSizeFromByteSize(c.mFields[i].mType.byteSize); // TODO: This won't work for nested structs
						outfile << "\tmov " << sizes[actualSize] << " [" << var.reg;
						if (var.offset > 0)
							outfile << "+" << var.offset - c.mFields[i].mOffset;
						else
							outfile << "-" << -(var.offset + c.mFields[i].mOffset);
						outfile << "], " << getRegister("a", actualSize) << std::endl;
					}
				} else {
					printExpression(outfile, p, v.mValues[0], 0);
					int size = addToSymbols(offset, v);
					addToSymbols(&localOffset, v);
					localSymbols.push_back(v.mName);
					outfile << "\tmov " << sizes[size] << " " << symbolTable[v.mName].location() << ", " << getRegister("a", size) << "; VAR_DECL_ASSIGN else" << std::endl;
				}

				break;
			}
			case Statement_Type::VAR_ASSIGNMENT: {
				Variable v = statement.variable.value();
				uint64_t index = v.mName.find('.');
				if (index != std::string::npos || statement.mContent != nullptr) {
					// Array or struct property
					if (v.mType.builtinType == Builtin_Type::ARRAY) {
						SymbolInfo& arr = symbolTable[v.mName];
						int actualSize = getSizeFromByteSize(arr.type.subTypes[0].byteSize);
						printExpression(outfile, p, statement.mContent, 0);
						// NOTE: I'm choosing to set the index to r11, which might get overwritten. Instead we could also do a push rax/pop r11
						// If unexpected behaviour is found, go with the more safe route of using the stack
						outfile << "\tmov r11, rax" << std::endl;

						printExpression(outfile, p, v.mValues[0], 0);
						outfile << "\tmov " << sizes[actualSize] << " [" << arr.reg;
						if (arr.offset > 0)
							outfile << "+" << arr.offset << "+r11*" << int(arr.type.subTypes[0].byteSize);
						else if (arr.offset < 0)
							outfile << "-" << -arr.offset <<  "+r11*" << int(arr.type.subTypes[0].byteSize);
						else
							outfile << "+r11*" << int(arr.type.subTypes[0].byteSize);
						outfile << "], " << getRegister("a", actualSize) << std::endl;

					} else if (v.mType.builtinType == Builtin_Type::STRUCT) {
						std::string propName = v.mName.substr(index + 1);
						std::string varName = v.mName.substr(0, index);
						SymbolInfo& var = symbolTable[varName];
						const Struct& s = p.structs.at(v.mType.name);
						int fieldIndex = s.getIndexOfProperty(propName);
						const StructField& sf = s.mFields[fieldIndex];
						int actualSize = getSizeFromByteSize(sf.mType.byteSize);

						printExpression(outfile, p, v.mValues[0], 0);
						outfile << "\tmov " << sizes[actualSize] << " [" << var.reg;
						if (var.offset > 0)
							outfile << "+" << var.offset - sf.mOffset;
						else
							outfile << "-" << -(var.offset + sf.mOffset);
						outfile << "], " << getRegister("a", actualSize) << std::endl;
					} else if (v.mType.builtinType == Builtin_Type::CLASS) {
						std::string propName = v.mName.substr(index + 1);
						std::string varName = v.mName.substr(0, index);
						SymbolInfo& var = symbolTable[varName];
						const Class& c = p.classes.at(v.mType.name);
						int fieldIndex = c.getIndexOfProperty(propName);
						const StructField& sf = c.mFields[fieldIndex];
						int actualSize = getSizeFromByteSize(sf.mType.byteSize);

						printExpression(outfile, p, v.mValues[0], 0);
						outfile << "\tmov " << sizes[actualSize] << " [" << var.reg;
						if (var.offset > 0)
							outfile << "+" << var.offset - sf.mOffset;
						else
							outfile << "-" << -(var.offset + sf.mOffset);
						outfile << "], " << getRegister("a", actualSize) << std::endl;
					}
				} else {
					// Redefinition
					if (v.mType.builtinType == Builtin_Type::ARRAY) {
						SymbolInfo& arr = symbolTable[v.mName];
						int actualSize = getSizeFromByteSize(arr.type.subTypes[0].byteSize);

						for (int i = 0; i < v.mValues.size(); i++) {
							printExpression(outfile, p, v.mValues[i], 0);
							outfile << "\tmov " << sizes[actualSize] << " [" << arr.reg;
							if (arr.offset > 0)
								outfile << "+" << arr.offset - i * int(arr.type.byteSize / v.mValues.size());
							else if (arr.offset <= 0)
								outfile << "-" << -(arr.offset + i * int(arr.type.byteSize / v.mValues.size()));
							outfile << "], " << getRegister("a", actualSize) << std::endl;
						}
					} else if (v.mType.builtinType == Builtin_Type::STRUCT) {
						const Struct& s = p.structs.at(v.mType.name);
						(*offset) -= int(s.mSize);
						localOffset -= int(s.mSize);
						addToSymbols(offset, v);
						addToSymbols(&localOffset, v);
						localSymbols.push_back(v.mName);
						SymbolInfo& var = symbolTable[v.mName];

						for (int i = 0; i < s.mFields.size(); i++) {
							Expression* exp = v.mValues.at(i);
							if (exp == nullptr) continue;
							printExpression(outfile, p, exp, 0);
							int actualSize = getSizeFromByteSize(s.mFields[i].mType.byteSize); // TODO: This won't work for nested structs
							outfile << "\tmov " << sizes[actualSize] << " [" << var.reg;
							if (var.offset > 0)
								outfile << "+" << var.offset - s.mFields[i].mOffset;
							else
								outfile << "-" << -(var.offset + s.mFields[i].mOffset);
							outfile << "], " << getRegister("a", actualSize) << std::endl;
						}
					}  else if (v.mType.builtinType == Builtin_Type::CLASS) {
						const Class& c = p.classes.at(v.mType.name);
						(*offset) -= int(c.mSize);
						localOffset -= int(c.mSize);
						addToSymbols(offset, v);
						addToSymbols(&localOffset, v);
						localSymbols.push_back(v.mName);
						SymbolInfo& var = symbolTable[v.mName];

						for (int i = 0; i < c.mFields.size(); i++) {
							Expression* exp = v.mValues.at(i);
							if (exp == nullptr) continue;
							printExpression(outfile, p, exp, 0);
							int actualSize = getSizeFromByteSize(c.mFields[i].mType.byteSize); // TODO: This won't work for nested structs
							outfile << "\tmov " << sizes[actualSize] << " [" << var.reg;
							if (var.offset > 0)
								outfile << "+" << var.offset - c.mFields[i].mOffset;
							else
								outfile << "-" << -(var.offset + c.mFields[i].mOffset);
							outfile << "], " << getRegister("a", actualSize) << std::endl;
						}
					} else {
						if (!symbolTable.contains(v.mName)) {
							// Class property
							printExpression(outfile, p, v.mValues[0], 0);
							const Class& klass = p.classes.at(currentClass);
							int propIndex = klass.getIndexOfProperty(v.mName);
							int leftSize = getSizeFromType(klass.mFields[propIndex].mType);
							outfile << "\tmov " << sizes[leftSize] << " [rdi+" << klass.mFields[propIndex].mOffset << "], " << getRegister("a", leftSize) << std::endl;
						} else {
							SymbolInfo& symbol = symbolTable[v.mName];
							printExpression(outfile, p, v.mValues[0], 0);
							bool dereferenceNeeded = symbol.reg == "rbp" || symbol.type.builtinType == Builtin_Type::REF;
							outfile << "\tmov " << sizes[symbol.size] << " " << symbol.location(dereferenceNeeded) << ", " << getRegister("a", symbol.size) << std::endl;
						}
					}
				}
			}
			case Statement_Type::LOOP: {
				if (!statement.loopStatement.has_value()) continue;
				LoopStatement ls = statement.loopStatement.value();
				if (ls.mIterator.has_value()) {
					int size = addToSymbols(offset, ls.mIterator.value());
					addToSymbols(&localOffset, ls.mIterator.value());
					localSymbols.push_back(ls.mIterator.value().mName);
					std::string op = ls.mRange.value().mMinimum < ls.mRange.value().mMaximum ? "inc " : "dec ";
					std::string label = ".label";
					uint32_t localLabelCount = ++labelCount;
					label = label.append(std::to_string(localLabelCount));
					recentLoopLabel = label;
					printExpression(outfile, p, ls.mRange.value().mMinimum, 0);
					const char* reg = getRegister("a", size);
					outfile << "\tmov " << sizes[size] << " " << symbolTable[ls.mIterator.value().mName].location() << ", " << reg << std::endl;
					outfile << label << ":" << std::endl;
					printExpression(outfile, p, ls.mRange.value().mMaximum, 0);
					outfile << "\tcmp " << sizes[size] << " " << symbolTable[ls.mIterator.value().mName].location() << ", " << reg << std::endl;
					outfile << "\tjne .inside_label" << localLabelCount << std::endl;
					outfile << "\tjmp .not_label" << localLabelCount << std::endl;
					outfile << ".inside_label" << localLabelCount << ":" << std::endl;
					printBody(outfile, p, ls.mBody, label, offset, allocs);
					outfile << ".skip_label" << localLabelCount << ":" << std::endl;
					outfile << "\t" << moveToRegister("rax", symbolTable[ls.mIterator.value().mName]).str();
					outfile << "\t" << op << "rax" << std::endl;
					outfile << "\tmov " << sizes[size] << " " << symbolTable[ls.mIterator.value().mName].location() << ", " << reg << std::endl;
					outfile << "\tjmp .label" << localLabelCount << std::endl;
					outfile << ".not_label" << localLabelCount << ":" << std::endl;
					symbolTable.erase(ls.mIterator.value().mName);
				} else {
					if (statement.mContent == nullptr) {
						// We have "loop { ... }"
						std::string label = ".label";
						label = label.append(std::to_string(++labelCount));
						outfile << label << ":" << std::endl;
						recentLoopLabel = label;
						printBody(outfile, p, ls.mBody, label, offset, allocs);
						outfile << "\tjmp .label" << labelCount << std::endl;
						outfile << ".not_label" << labelCount << ":" << std::endl; // Used for break statements
					} else {
						// we have "until condition { ... }"
					}
				}
				break;
			}
			case Statement_Type::BREAK:
				if (!recentLoopLabel.empty())
					outfile << "\tjmp .not_" << recentLoopLabel.substr(1) << std::endl;
				break;
			case Statement_Type::SKIP:
				if (!recentLoopLabel.empty())
					outfile << "\tjmp .skip_" << recentLoopLabel.substr(1) << std::endl;
				break;
			case Statement_Type::FUNC_CALL: {
				FuncCallStatement fc = statement.funcCall.value();
				// If function is stdlib call, need to expand this into something better when stdlib expands
				if (labelName != "main") {
					outfile << "\tpush rdi" << std::endl;
					outfile << "\tpush rsi" << std::endl;
					outfile << "\tpush rdx" << std::endl;
					outfile << "\tpush rcx" << std::endl;
					outfile << "\tpush r8" << std::endl;
					outfile << "\tpush r9" << std::endl;
					outfile << "\tpush r10" << std::endl;
				}
				if (fc.mClassName == "stdout" || fc.mClassName == "stdin") {
					printFunctionCall(outfile, p, fc);
				} else {
					if (fc.mArgs.empty() && !fc.mClassName.empty()) {
						const SymbolInfo& symbol = symbolTable[fc.mClassName];
						outfile << "\tlea rax, " << symbol.location(true) << std::endl;
						outfile << "\tmov " << callingConvention[0] << ", rax" << std::endl;
					} else {
						for (int i = fc.mArgs.size() - 1; i >= 0; i--) {
							if (i == 0 && !fc.mClassName.empty()) {
								const SymbolInfo& symbol = symbolTable[fc.mClassName];
								outfile << "\tlea rax, " << symbol.location(true) << std::endl;
								outfile << "\tmov " << callingConvention[0] << ", rax" << std::endl;
								continue;
							}
							std::string value;
							Expression* expr = fc.mArgs[i];
							if (expr->mValue.mSubType == TokenSubType::STRING_LITERAL) {
								value = p.findLiteralByContent(expr->mValue.mText)->mAlias;
							} else {
								printExpression(outfile, p, expr, 0);
								value = "rax";
							}
							if (i <= 6)
								outfile << "\tmov " << callingConvention[i] << ", " << value << std::endl;
							else
								outfile << "\tpush " << value << std::endl;
						}
					}
					if (fc.mFunctionName == "printf" && fc.mIsExternal) {
						outfile << "\tmov rax, 0" << std::endl;
					}
					if (fc.mFunctionName.substr(0, 4) == "SYS_") {
						printSyscall(outfile, fc.mFunctionName);
						outfile << "\tsyscall" << std::endl;
					} else {
						outfile << "\tcall ";
						if (!fc.mNamespace.empty())
							outfile << fc.mNamespace << "_";
						if (!fc.mClassName.empty()) {
							const SymbolInfo& symbol = symbolTable[fc.mClassName];
							outfile << symbol.type.name << "_";
						}
						outfile << statement.funcCall.value().mFunctionName << std::endl;
					}
				}
				if (labelName != "main") {
					outfile << "\tpop r10" << std::endl;
					outfile << "\tpop r9" << std::endl;
					outfile << "\tpop r8" << std::endl;
					outfile << "\tpop rcx" << std::endl;
					outfile << "\tpop rdx" << std::endl;
					outfile << "\tpop rsi" << std::endl;
					outfile << "\tpop rdi" << std::endl;
				}
				break;
			}
			case Statement_Type::NOTHING:
				outfile << "\tnop" << std::endl;
				break;
			case Statement_Type::IF: {
				// TODO: If statements with a function call for expression don't work
				IfStatement is = statement.ifStatement.value();
				printExpression(outfile, p, statement.mContent, 0);
				std::string label = ".if";
				uint32_t localIfCount = ++ifCount;
				label = label.append(std::to_string(localIfCount));
				outfile << "\ttest rax, rax" << std::endl;
				outfile << "\tjnz " << label << std::endl;
				if (is.mElse.has_value())
					outfile << "\tjmp .else_if" <<  localIfCount << std::endl;
				else
					outfile << "\tjmp .end_if" << localIfCount << std::endl;
				outfile << label << ":" << std::endl;
				printBody(outfile, p, is.mBody, label, offset, allocs);
				outfile << "\tjmp .end_if" << localIfCount << std::endl;
				if (is.mElse.has_value()) {
					outfile << ".else_if" << localIfCount << ":" << std::endl;
					printBody(outfile, p, is.mElseBody.value(), label, offset, allocs);
				}
				outfile << ".end_if" << localIfCount << ":" << std::endl;
				break;
			}
			case Statement_Type::ARRAY_INDEX:
				outfile << "; Array indexing compiled. This shouldn't ever happen!" << std::endl;
				break;
		}
	}

	if (block.stackMemory != 0) {
		outfile << "\tadd rsp, " << block.stackMemory + block.biggestAlloc << std::endl;
		(*allocs) -= (block.stackMemory + block.biggestAlloc);
	}

	for (const auto& symbolName : localSymbols) {
		symbolTable.erase(symbolName);
	}
}

std::stringstream X86_64LinuxYasmCompiler::moveToRegister(const std::string& reg, const SymbolInfo& symbol) {
	const char* sizes[] = {"byte", "word", "dword", "qword"};
	std::stringstream ss;
	switch (symbol.type.builtinType) {
		case Builtin_Type::UI8:
		case Builtin_Type::UI16:
			ss << "movzx ";
			break;
		case Builtin_Type::I8:
		case Builtin_Type::I16:
			ss << "movsx ";
			break;
		case Builtin_Type::I32:
			ss << "movsxd ";
			break;
		case Builtin_Type::UI32:
		case Builtin_Type::UI64:
		case Builtin_Type::I64:
		default:
			ss << "mov ";
			break;
	}
	ss << reg << ", " << sizes[symbol.size] << " " << symbol.location() << std::endl;
	return ss;
}

ExpressionPrinted X86_64LinuxYasmCompiler::printExpression(std::ofstream& outfile, const Programme& p, const Expression* expression, uint8_t nodeType) {
	if (expression == nullptr) return ExpressionPrinted{};

	const char* sizes[] = {"byte", "word", "dword", "qword"};

	if (nodeType == 0 && expression->mChildren.empty()) {
		// No expression, just base value
		if (expression->mValue.mSubType == TokenSubType::STRING_LITERAL) {
			outfile << "\tmov rax, " << p.findLiteralByContent(expression->mValue.mText).value().mAlias << std::endl;
		} else if (expression->mValue.mSubType == TokenSubType::INTEGER_LITERAL) {
			outfile << "\tmov rax, " << expression->mValue.mText << std::endl;
		} else if (expression->mValue.mType == TokenType::IDENTIFIER) {
			SymbolInfo& val = symbolTable[expression->mValue.mText];
			bool sign = val.type.name[0] == 'i'; // This might cause a problem later with user-defined types starting with i
			if (val.size == 2) {// dword
				outfile << "\tmov rax, 0" << std::endl;
				outfile << "\tmov eax, " << sizes[val.size] << " " << val.location() << std::endl;
			} else if (val.isGlobal) {
				outfile << "\tmov rax, " << val.location(false) << std::endl;
			} else {
				const char* moveAction = getMoveAction(3, val.size, sign);
				outfile << "\t" << moveAction << " rax, " << sizes[val.size] << " " << val.location() << std::endl;
			}
		} else if (expression->mValue.mType == TokenType::OPERATOR) {
			if (expression->mValue.mText == "++") {
				outfile << "\tinc rax" << std::endl;
			} else if (expression->mValue.mText == "--") {
				outfile << "\tdec rax" << std::endl;
			}
		}
	}
	if (expression->mChildren.empty())
		return ExpressionPrinted{};

	if (expression->mChildren[0] == nullptr && expression->mChildren[1] == nullptr) {
		return ExpressionPrinted{};
	}
	if (expression->mValue.mType != TokenType::OPERATOR) {
		return ExpressionPrinted{};
	}
	if (expression->mChildren[0]->mValue.mSubType == TokenSubType::STRING_LITERAL) {
		return ExpressionPrinted{};
	}

	ExpressionPrinted curr = {};
	int leftSize;
	bool leftSign = false;
	int rightSize;
	bool rightSign = false;

	if (expression->mValue.mText == "[") {
		// blep = arr[i]
		//         [
		//      arr  i
		printExpression(outfile, p, expression->mChildren[1], 0);

		SymbolInfo& arr = symbolTable[expression->mChildren[0]->mValue.mText];
		// NOTE: Isn't only arrays, but can also be refs, strings, or if we want, numbers indexed to the bits
		int actualSize = getSizeFromByteSize(arr.type.subTypes[0].byteSize);
		if (arr.type.builtinType == Builtin_Type::ARRAY) {
			bool sign = arr.type.subTypes[0].name[0] == 'i'; // This might cause a problem later with user-defined types starting with i
			const char* moveAction = getMoveAction(3, actualSize, sign);
			outfile << "\t" << moveAction << " r10, " << sizes[actualSize] << " [" << arr.reg;
			if (arr.offset > 0)
				outfile << "+" << arr.offset << "+rax*" << int(arr.type.subTypes[0].byteSize);
			else if (arr.offset < 0)
				outfile << "-" << -arr.offset << "+rax*" << int(arr.type.subTypes[0].byteSize);
			else
				outfile << "+rax*" << int(arr.type.subTypes[0].byteSize);
			outfile << "]" << std::endl;
			outfile << "\tmov rax, r10" << std::endl;
		} else if (arr.type.builtinType == Builtin_Type::REF) {
			const char* moveAction = getMoveAction(3, actualSize, false);
			outfile << "\tmov rbx, " << arr.type.subTypes[0].byteSize << std::endl;
			outfile << "\tmul rbx" << std::endl;
			outfile << "\tmov rbx, qword [" << arr.reg;
			if (arr.offset > 0)
				outfile << "+" << arr.offset;
			else if (arr.offset < 0)
				outfile << "-" << -arr.offset;
			outfile << "]" << std::endl;
			outfile << "\tadd rax, rbx" << std::endl;
			outfile << "\t" << moveAction << " r10, " <<  sizes[actualSize] << " [rax]" << std::endl;
			outfile << "\tmov rax, r10" << std::endl;
		}
		return ExpressionPrinted{};
	} else if (expression->mValue.mText == "(") {
		const char callingConvention[6][4] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
		std::stringstream ss;
		bool printArgs = false;
		int i = 0;
		outfile << "\tpush rdi" << std::endl;
		outfile << "\tpush rsi" << std::endl;
		outfile << "\tpush rdx" << std::endl;
		outfile << "\tpush rcx" << std::endl;
		outfile << "\tpush r8" << std::endl;
		outfile << "\tpush r9" << std::endl;
		outfile << "\tpush r10" << std::endl;
		std::string classVariable = "";
		for (const auto& child : expression->mChildren) {
			if (child->mValue.mText == "e" || child->mValue.mText == ":") continue;
			if (child->mValue.mText == "(") {
				printArgs = true;
				if (!classVariable.empty()) {
					const SymbolInfo& symbol = symbolTable[classVariable];
					outfile << "\tlea rax, " << symbol.location(true) << std::endl;
					outfile << "\tmov " << callingConvention[0] << ", rax" << std::endl;
					i++;
				}
				continue;
			}
			if (!printArgs) {
				if (child->mValue.mType == TokenType::OPERATOR)
					ss << "_";
				else {
					if (!symbolTable.contains(child->mValue.mText)) {
						ss << child->mValue.mText;
						continue;
					}
					const SymbolInfo& symbol = symbolTable[child->mValue.mText];
					ss << symbol.type.name;
					classVariable = child->mValue.mText;
				}
			} else {
				std::string value;

				if (child->mValue.mSubType == TokenSubType::STRING_LITERAL) {
					value = p.findLiteralByContent(child->mValue.mText)->mAlias;
				} else {
					printExpression(outfile, p, child, 0);
					value = "rax";
				}
				if (i <= 6)
					outfile << "\tmov " << callingConvention[i] << ", " << value << std::endl;
				else
					outfile << "\tpush " << value << std::endl; // TODO: This is a bug, values should be pushed onto the stack in reverse order
				i++;
			}
		}
		if (ss.str() == "printf") {
			outfile << "\tmov rax, 0" << std::endl;
		}
		if (ss.str().substr(0, 4) == "SYS_") {
			printSyscall(outfile, ss.str());
			outfile << "\tsyscall" << std::endl;
		} else {
			outfile << "\tcall " << ss.str() << std::endl;
		}
		if (nodeType == 1) {
			outfile << "\tmov rbx, rax; printExpression, nodeType=1, function call" << std::endl;
		}

		outfile << "\tpop r10" << std::endl;
		outfile << "\tpop r9" << std::endl;
		outfile << "\tpop r8" << std::endl;
		outfile << "\tpop rcx" << std::endl;
		outfile << "\tpop rdx" << std::endl;
		outfile << "\tpop rsi" << std::endl;
		outfile << "\tpop rdi" << std::endl;

		return ExpressionPrinted{ true, false, 3 };
	} else if (expression->mValue.mSubType == TokenSubType::OP_UNARY) {
		if (expression->mValue.mText == "\\") {
			Expression* child = expression->mChildren[0];
			if (child->mValue.mType == TokenType::IDENTIFIER) {
				SymbolInfo& left = symbolTable[expression->mChildren[0]->mValue.mText];
				leftSize = left.size;
				if (left.reg != "rbp")
					outfile << "\tlea rax, " << left.location(false) << std::endl;
				else
					outfile << "\tlea rax, " << left.location(true) << std::endl;
			} else if (child->mValue.mSubType == TokenSubType::STRING_LITERAL) {
				outfile << "\tlea rax, " << p.findLiteralByContent(child->mValue.mText).value().mAlias << std::endl;
			} else if (child->mValue.mType == TokenType::LITERAL) {
				throw std::runtime_error("Cannot get reference of a literal value");
			} else {
				throw std::runtime_error("Unexpected getting a reference");
			}
		} else if (expression->mValue.mText == "@") {
			ExpressionPrinted valPrinted = printExpression(outfile, p, expression->mChildren[0], -1);
			if (!valPrinted.printed) {
				Expression* child = expression->mChildren[0];
				if (child->mValue.mType == TokenType::IDENTIFIER) {
					SymbolInfo& left = symbolTable[expression->mChildren[0]->mValue.mText];
					leftSign = left.type.name[0] == 'i'; // This might cause a problem later with user-defined types starting with i
					leftSize = left.size;
					const char* reg = "r10";
					const char* moveAction = getMoveAction(3, leftSize, leftSign);
					outfile << "\t" << moveAction << " " << reg << ", " << sizes[left.size] << " " << left.location() << std::endl;
					outfile << "\tmov rax, [r10]" << std::endl;
				} else if (child->mValue.mSubType == TokenSubType::STRING_LITERAL) {
					throw std::runtime_error("Cannot dereference a string literal value");
				} else if (child->mValue.mType == TokenType::LITERAL) {
					outfile << "\tmov r10, " << child->mValue.mText << std::endl;
					outfile << "\tmov rax, [r10]" << std::endl;
				} else {
					throw std::runtime_error("Unexpected dereference");
				}
			} else {
				outfile << "\tmov r10, rax" << std::endl;
				outfile << "\tmov rax, [r10]" << std::endl;
			}
		} else if (expression->mValue.mText == "!") {
			ExpressionPrinted valPrinted = printExpression(outfile, p, expression->mChildren[0], -1);
			if (valPrinted.printed) {
				// NOTE: If we knew the type that is in rax, we could possibly only do a xor, which is a bit more efficient
				outfile << "\tcmp rax, 0" << std::endl;
				outfile << "\tsete al" << std::endl;
				outfile << "\tmovzx rax, al" << std::endl;
			} else {
				Expression* child = expression->mChildren[0];
				if (child->mValue.mType == TokenType::IDENTIFIER) {
					SymbolInfo& left = symbolTable[expression->mChildren[0]->mValue.mText];
					leftSign = left.type.name[0] == 'i'; // This might cause a problem later with user-defined types starting with i
					leftSize = left.size;
					if (left.type.builtinType == Builtin_Type::BOOL) {
						const char* reg = "rax";
						const char* moveAction = getMoveAction(3, leftSize, leftSign);
						outfile << "\t" << moveAction << " " << reg << ", " << sizes[left.size] << " " << left.location() << std::endl;
						outfile << "\txor rax, 1" << std::endl;
					} else {
						outfile << "\tcmp " << sizes[left.size] << " " << left.location() << ", 0" << std::endl;
						outfile << "\tsete al" << std::endl;
						outfile << "\tmovzx rax, al" << std::endl;
					}
				} else if (child->mValue.mSubType == TokenSubType::STRING_LITERAL) {
					throw std::runtime_error("Cannot `!` a string literal value");
				} else if (child->mValue.mType == TokenType::LITERAL) {
					outfile << "\tmov rax, " << child->mValue.mText << std::endl;
					if (child->mValue.mSubType == TokenSubType::BOOLEAN_LITERAL)
						outfile << "\txor rax, 1" << std::endl;
					// Else, flip all the bits. If someone wants to use ints as bools, they will have to `!intVal & 1` instead of `!intVal`.
					else {
						outfile << "\tcmp rax, 0" << std::endl;
						outfile << "\tsete al" << std::endl;
						outfile << "\tmovzx rax, al" << std::endl;
					}
				} else {
					throw std::runtime_error("Unexpected `!`");
				}
			}
		} else if (expression->mValue.mText == "~") {
			ExpressionPrinted valPrinted = printExpression(outfile, p, expression->mChildren[0], -1);
			if (valPrinted.printed)
				outfile << "\tnot rax" << std::endl;
			else {
				Expression* child = expression->mChildren[0];
				if (child->mValue.mType == TokenType::IDENTIFIER) {
					SymbolInfo& left = symbolTable[expression->mChildren[0]->mValue.mText];
					leftSign = left.type.name[0] == 'i'; // This might cause a problem later with user-defined types starting with i
					leftSize = left.size;
					const char* reg = "rax";
					const char* moveAction = getMoveAction(3, leftSize, leftSign);
					outfile << "\t" << moveAction << " " << reg << ", " << sizes[left.size] << " " << left.location() << std::endl;
					outfile << "\tnot rax" << std::endl;
				} else if (child->mValue.mSubType == TokenSubType::STRING_LITERAL) {
					throw std::runtime_error("Cannot `~` a string literal value");
				} else if (child->mValue.mType == TokenType::LITERAL) {
					outfile << "\tmov rax, " << child->mValue.mText << std::endl;
					outfile << "\tnot rax" << std::endl;
				} else {
					throw std::runtime_error("Unexpected `!`");
				}
			}
		}
		if (nodeType == 1) {
			outfile << "\tmov rbx, rax; printExpression, nodeType=1, unary op" << std::endl;
		}
		return ExpressionPrinted{ true, false, 3 };
	} else if (expression->mValue.mText == ".") {
		// We have a struct property
		SymbolInfo& left = symbolTable[expression->mChildren[0]->mValue.mText]; // Left is variable name
		std::string propName = expression->mChildren[1]->mValue.mText; // Right is property name
		std::string structName = left.type.name;
		// Left can be a ref<T> with subtype struct
		if (!left.type.subTypes.empty()) {
			structName = left.type.subTypes[0].name; // TODO: We assume only one level deep
		}
		int actualSize = 0;
		if (left.type.builtinType == Builtin_Type::STRUCT) {
			const Struct& s = p.structs.at(structName);
			int fieldIndex = s.getIndexOfProperty(propName);
			const StructField& sf = s.mFields[fieldIndex];
			actualSize = getSizeFromByteSize(sf.mType.byteSize);
			const char* moveAction = getMoveAction(3, actualSize, sf.mType.name[0] == 'i');

			outfile << "\t" << moveAction << " " << getRegister("a", actualSize >= 2 ? actualSize : 3) << ", " << sizes[actualSize] << " [" << left.reg;
			if (left.reg != "rbp") {
				outfile << "+" << left.offset + sf.mOffset;
			} else {
				if (left.offset > 0)
					outfile << "+" << int(left.offset - sf.mOffset);
				else
					outfile << "-" << -(int(left.offset + sf.mOffset));
			}
			outfile << "]" << std::endl;
			if (nodeType == 1) {
				outfile << "\tmov rbx, rax; printExpression, nodeType=1, struct property" << std::endl;
			}
		} else {
			// Class
			const Class& c = p.classes.at(structName);
			int fieldIndex = c.getIndexOfProperty(propName);
			const StructField& sf = c.mFields[fieldIndex];
			actualSize = getSizeFromByteSize(sf.mType.byteSize);
			const char* moveAction = getMoveAction(3, actualSize, sf.mType.name[0] == 'i');

			outfile << "\t" << moveAction << " " << getRegister("a", actualSize >= 2 ? actualSize : 3) << ", " << sizes[actualSize] << " [" << left.reg;
			if (left.reg != "rbp") {
				outfile << "+" << left.offset + sf.mOffset;
			} else {
				if (left.offset > 0)
					outfile << "+" << int(left.offset - sf.mOffset);
				else
					outfile << "-" << -(int(left.offset + sf.mOffset));
			}
			outfile << "]" << std::endl;
			if (nodeType == 1) {
				outfile << "\tmov rbx, rax; printExpression, nodeType=1, class property" << std::endl;
			}
		}
		return ExpressionPrinted{ true, false, actualSize };
	}

	ExpressionPrinted leftPrinted = printExpression(outfile, p, expression->mChildren[0], -1);
	if (leftPrinted.printed) {
		std::string r1 = "r10";//getRegister("10", leftPrinted.size);
		std::string r2 = "rax";//getRegister("a", leftPrinted.size);
		outfile << "\tmov " << r1 << ", " << r2 << std::endl; // Save left
	}
	ExpressionPrinted rightPrinted = printExpression(outfile, p, expression->mChildren[1], 1);

	if (expression->mChildren[0]->mValue.mType == TokenType::IDENTIFIER) {
		if (!symbolTable.contains(expression->mChildren[0]->mValue.mText)) {
			// Class member
			const Class& klass = p.classes.at(currentClass);
			int propIndex = klass.getIndexOfProperty(expression->mChildren[0]->mValue.mText);
			leftSign = klass.mFields[propIndex].mType.name[0] == 'i';
			leftSize = getSizeFromType(klass.mFields[propIndex].mType);
			const char* reg = leftSize < 2 ? "rax" : getRegister("a", leftSize);
			const char* moveAction = getMoveAction(3, leftSize, leftSign);
			outfile << "\t" << moveAction << " " << reg << ", " << sizes[leftSize] << " [rdi+" << klass.mFields[propIndex].mOffset << "]" << "; printExpression, left identifier, class property" << std::endl;
		} else {
			SymbolInfo& left = symbolTable[expression->mChildren[0]->mValue.mText];
			leftSign = left.type.name[0] == 'i'; // This might cause a problem later with user-defined types starting with i
			leftSize = left.size;
			const char* reg = "rax";//getRegister("a", left.size);
			const char* moveAction = getMoveAction(3, leftSize, leftSign);
			if (left.reg == "rbp")
				outfile << "\t" << moveAction << " " << reg << ", " << sizes[left.size] << " " << left.location() << "; printExpression, left identifier, rbp" << std::endl;
			else
				outfile << "\t" << moveAction << " " << reg << ", " << left.location(false) << "; printExpression, left identifier, not rbp" << std::endl;
		}
	} else if (expression->mChildren[0]->mValue.mSubType == TokenSubType::INTEGER_LITERAL) {
		int size = getSizeFromNumber(expression->mChildren[0]->mValue.mText);
		if (size < 0) {
			leftSign = true;
			size = -size - 1;
		}
		leftSize = size;
		const char* reg = "rax";//getRegister("a", size);
		outfile << "\tmov " << reg << ", " << expression->mChildren[0]->mValue.mText << "; printExpression, left int" << std::endl;
	} else {
		leftSize = leftPrinted.size;
	}

	if (expression->mChildren[1]->mValue.mType == TokenType::IDENTIFIER) {
		if (!symbolTable.contains(expression->mChildren[1]->mValue.mText)) {
			const Class& klass = p.classes.at(currentClass);
			int propIndex = klass.getIndexOfProperty(expression->mChildren[1]->mValue.mText);
			rightSign = klass.mFields[propIndex].mType.name[0] == 'i';
			rightSize = getSizeFromType(klass.mFields[propIndex].mType);
			const char* reg = rightSize < 2 ? "rbx" : getRegister("b", rightSize);
			const char* moveAction = getMoveAction(3, rightSize, rightSign);
			outfile << "\t" << moveAction << " " << reg << ", " << sizes[rightSize] << " [rdi+" << klass.mFields[propIndex].mOffset << "]" << "; printExpression, right identifier, class property" << std::endl;
		} else {
			SymbolInfo& right = symbolTable[expression->mChildren[1]->mValue.mText];
			rightSign = right.type.name[0] == 'i'; // This might cause a problem later with user-defined types starting with i
			rightSize = right.size;
			const char* reg = "rbx";//getRegister("b", right.size);
			const char* moveAction = getMoveAction(3, rightSize, rightSign);
			if (right.reg == "rbp")
				outfile << "\t" << moveAction << " " << reg << ", " << sizes[right.size] << " " << right.location() << "; printExpression, right identifier, rbp" << std::endl;
			else
				outfile << "\t" << moveAction << " " << reg << ", " << right.location(false) << "; printExpression, right identifier, not rbp" << std::endl;
		}
	} else if (expression->mChildren[1]->mValue.mSubType == TokenSubType::INTEGER_LITERAL) {
		int size = getSizeFromNumber(expression->mChildren[1]->mValue.mText);
		if (size < 0) {
			rightSize = true;
			size = -size - 1;
		}
		rightSize = size;
		const char* reg = "rbx";//getRegister("b", size);
		outfile << "\tmov " << reg << ", " << expression->mChildren[1]->mValue.mText << "; printExpression, right int" << std::endl;
	} else {
		rightSize = rightPrinted.size;
	}

	curr.sign = leftPrinted.sign || rightPrinted.sign || leftSign || rightSign;

	if (leftPrinted.printed) {
		std::string r1 = "rax";//getRegister("a", leftPrinted.size);
		std::string r2 = "r10";//getRegister("10", leftPrinted.size);
		outfile << "\tmov " << r1 << ", " << r2 << "; printExpression, leftPrinted, recover left" << std::endl; // Recover left
	}

	if (expression->mValue.mText == "+") {
		int size = getEvenSize(leftSize, rightSize);
		std::string r1 = getRegister("a", size);
		std::string r2 = getRegister("b", size);
		outfile << "\tadd " << r1 << ", " << r2 << std::endl;
	} else if (expression->mValue.mText == "-") {
		int size = getEvenSize(leftSize, rightSize);
		if (size == 3)
			outfile << "\tsub rax, rbx" << std::endl;
		else {
			std::string r1 = getRegister("a", size+1);
			std::string r2 = getRegister("b", size+1);
			outfile << "\tsub " << r1 << ", " << r2 << std::endl;
		}
	} else if (expression->mValue.mText == "*") {
		int size = getEvenSize(leftSize, rightSize);
		std::string r2 = getRegister("b", size);
		if (curr.sign)
			outfile << "\timul " << sizes[size] << " " << r2 << std::endl;
		else
			outfile << "\tmul " << sizes[size] << " " << r2 << std::endl;
	} else if (expression->mValue.mText == "/") {
		int size = getEvenSize(leftSize, rightSize);
		std::string r1 = getRegister("a", size);
		std::string r2 = getRegister("b", size);
		outfile << "\t" << convertARegSize(size) << std::endl;
		if (curr.sign)
			outfile << "\tidiv " << r1 << ", " << r2 << std::endl;
		else
			outfile << "\tdiv " << r1 << ", " << r2 << std::endl;
	} else if (expression->mValue.mText == "%") {
		int size = getEvenSize(leftSize, rightSize);
		std::string r1 = getRegister("a", size);
		std::string r2 = getRegister("b", size);
		std::string r3 = size > 0 ? getRegister("d", size) : "ah";
		outfile << "\t" << convertARegSize(size) << std::endl;
		if (curr.sign)
			outfile << "\tidiv " << r1 << ", " << r2 << std::endl;
		else
			outfile << "\tdiv " << r1 << ", " << r2 << std::endl;
		outfile << "\tmov " << r1 << ", " << r3 << std::endl; // Remainder of div (mod) is stored in d-register
	} else if (expression->mValue.mText == "<") {
		printConditionalMove(outfile, leftSize, rightSize, "cmovl");
	} else if (expression->mValue.mText == "<=") {
		printConditionalMove(outfile, leftSize, rightSize, "cmovle");
	} else if (expression->mValue.mText == "==") {
		printConditionalMove(outfile, leftSize, rightSize, "cmove");
	} else if (expression->mValue.mText == ">") {
		printConditionalMove(outfile, leftSize, rightSize, "cmovg");
	} else if (expression->mValue.mText == ">=") {
		printConditionalMove(outfile, leftSize, rightSize, "cmovge");
	} else if (expression->mValue.mText == "&") {
		int size = getEvenSize(leftSize, rightSize);
		std::string r1 = getRegister("a", size);
		std::string r2 = getRegister("b", size);
		outfile << "\tand " << r1 << ", " << r2 << std::endl;
	} else if (expression->mValue.mText == "|") {
		int size = getEvenSize(leftSize, rightSize);
		std::string r1 = getRegister("a", size);
		std::string r2 = getRegister("b", size);
		outfile << "\tor " << r1 << ", " << r2 << std::endl;
	} else if (expression->mValue.mText == "^") {
		int size = getEvenSize(leftSize, rightSize);
		std::string r1 = getRegister("a", size);
		std::string r2 = getRegister("b", size);
		outfile << "\txor " << r1 << ", " << r2 << std::endl;
	} else if (expression->mValue.mText == ">>") {
		std::string r2 = getRegister("c", 0);
		outfile << "\tmov rcx, rbx; printExpression, shift right" << std::endl;
		outfile << "\tshr rax" << ", " << r2 << std::endl;
	} else if (expression->mValue.mText == "<<") {
		std::string r2 = getRegister("c", 0);
		outfile << "\tmov rcx, rbx; printExpression, shift left" << std::endl;
		outfile << "\tshl rax" << ", " << r2 << std::endl;
	}

	if (nodeType == 1) {
		std::string r1 = getRegister("b", leftSize);
		std::string r2 = getRegister("a", rightSize);
		outfile << "\tmov rbx, rax; printExpression, nodeType=1" << std::endl;
	}

	curr.size = getEvenSize(leftSize, rightSize);
	curr.printed = true;

	return curr;
}

void X86_64LinuxYasmCompiler::printConditionalMove(std::ofstream& outfile, int leftSize, int rightSize, const char* instruction) {
	int size = getEvenSize(leftSize, rightSize);
	std::string r1 = getRegister("a", size);
	std::string r2 = getRegister("b", size);
	outfile << "\tmov rcx, 0" << std::endl;
	outfile << "\tmov rdx, 1" << std::endl;
	outfile << "\tcmp " << r1 << ", " << r2 << std::endl;
	outfile << "\t" << instruction << " rcx, rdx" << std::endl;
	outfile << "\tmov rax, rcx; printConditionalMove" << std::endl;
}

const char* X86_64LinuxYasmCompiler::getRegister(const std::string& reg, int size) {
	if (size == 0) {
		if (reg == "a") return "al";
		if (reg == "b") return "bl";
		if (reg == "c") return "cl";
		if (reg == "d") return "dl";
		if (reg == "si") return "sil";
		if (reg == "di") return "dil";
		if (reg == "bp") return "bpl";
		if (reg == "sp") return "spl";
		if (reg == "8") return "r8b";
		if (reg == "9") return "r9b";
		if (reg == "10") return "r10b";
		if (reg == "11") return "r11b";
		if (reg == "12") return "r12b";
		if (reg == "13") return "r13b";
		if (reg == "14") return "r14b";
		if (reg == "15") return "r15b";
	} else if (size == 1) {
		if (reg == "a") return "ax";
		if (reg == "b") return "bx";
		if (reg == "c") return "cx";
		if (reg == "d") return "dx";
		if (reg == "si") return "si";
		if (reg == "di") return "di";
		if (reg == "bp") return "bp";
		if (reg == "sp") return "sp";
		if (reg == "8") return "r8w";
		if (reg == "9") return "r9w";
		if (reg == "10") return "r10w";
		if (reg == "11") return "r11w";
		if (reg == "12") return "r12w";
		if (reg == "13") return "r13w";
		if (reg == "14") return "r14w";
		if (reg == "15") return "r15w";
	} else if (size == 2) {
		if (reg == "a") return "eax";
		if (reg == "b") return "ebx";
		if (reg == "c") return "ecx";
		if (reg == "d") return "edx";
		if (reg == "si") return "esi";
		if (reg == "di") return "edi";
		if (reg == "bp") return "ebp";
		if (reg == "sp") return "esp";
		if (reg == "8") return "r8d";
		if (reg == "9") return "r9d";
		if (reg == "10") return "r10d";
		if (reg == "11") return "r11d";
		if (reg == "12") return "r12d";
		if (reg == "13") return "r13d";
		if (reg == "14") return "r14d";
		if (reg == "15") return "r15d";
	} else if (size == 3) {
		if (reg == "a") return "rax";
		if (reg == "b") return "rbx";
		if (reg == "c") return "rcx";
		if (reg == "d") return "rdx";
		if (reg == "si") return "rsi";
		if (reg == "di") return "rdi";
		if (reg == "bp") return "rbp";
		if (reg == "sp") return "rsp";
		if (reg == "8") return "r8";
		if (reg == "9") return "r9";
		if (reg == "10") return "r10";
		if (reg == "11") return "r11";
		if (reg == "12") return "r12";
		if (reg == "13") return "r13";
		if (reg == "14") return "r14";
		if (reg == "15") return "r15";
	}
	return "UNREACHABLE";
}

int X86_64LinuxYasmCompiler::getSizeFromNumber(const std::string& text) {
	int64_t value = atol(text.c_str());
	if (value < 0) {
		// Has to be signed
		if (value >= -128) {
			return -1;
		} else if (value >= -32768) {
			return -2;
		} else if (value >= -2147483648) {
			return -3;
		} else {
			return -4;
		}
	} else {
		if (value <= 255) {
			return 0;
		} else if (value <= 65535) {
			return 1;
		} else if (value <= 4294967295) {
			return 2;
		} else {
			return 3;
		}
	}
}

int X86_64LinuxYasmCompiler::getEvenSize(int size1, int size2) {
	if (size1 == 0 || size2 > size1) return size2;
	else return size1;
}

const char* X86_64LinuxYasmCompiler::getMoveAction(int regSize, int valSize, bool isSigned) {
	if (regSize - valSize >= 2) { // rax with byte, word, but not dword or qword
		if (isSigned) return "movsx";
		else return "movzx";
	} else {
		if (isSigned) return "movsxd";
		else return "mov";
	}
}

int X86_64LinuxYasmCompiler::getSizeFromByteSize(size_t byteSize) {
	switch (byteSize) {
		case 1: return 0;
		case 2: return 1;
		case 4: return 2;
		case 8: return 3;
		default: return -1;
	}
}

int X86_64LinuxYasmCompiler::getSizeFromType(const Type& type) {
	switch (type.builtinType) {
		case Builtin_Type::UNDEFINED:
		case Builtin_Type::VOID:
		case Builtin_Type::STRUCT:
		case Builtin_Type::ARRAY:
		case Builtin_Type::CLASS:
			break;
		case Builtin_Type::UI8:
		case Builtin_Type::I8:
		case Builtin_Type::F8:
		case Builtin_Type::CHAR:
			return 0;
		case Builtin_Type::UI16:
		case Builtin_Type::I16:
		case Builtin_Type::F16:
			return 1;
		case Builtin_Type::UI32:
		case Builtin_Type::I32:
		case Builtin_Type::F32:
		case Builtin_Type::BOOL:
			return 2;
		case Builtin_Type::UI64:
		case Builtin_Type::I64:
		case Builtin_Type::F64:
		case Builtin_Type::REF:
			return 3;
	}
	if (type.name == "string")
		return 3;
	return 0;
}


const char* X86_64LinuxYasmCompiler::convertARegSize(int size) {
	if (size == 0) return "cbw";
	if (size == 1) return "cwd";
	if (size == 2) return "cdq";
	if (size == 3) return "cqo";
	return "UNREACHABLE";
}

const char* X86_64LinuxYasmCompiler::getDefineBytes(size_t byteSize) {
	switch (byteSize) {
		case 1: return "db";
		case 2: return "dw";
		case 4: return "dd";
		case 8: return "dq";
		case 16: return "ddq";
	}
	return "UNREACHABLE";
}

const char* X86_64LinuxYasmCompiler::getReserveBytes(size_t byteSize) {
	switch (byteSize) {
		case 1: return "resb";
		case 2: return "resw";
		case 4: return "resd";
		case 8: return "resq";
		case 16: return "resdq";
	}
	return "UNREACHABLE";
}
