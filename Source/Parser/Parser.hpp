#ifndef FOREST_PARSER_HPP
#define FOREST_PARSER_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include "Tokeniser.hpp"

namespace forest::parser {
	enum Builtin_Type {
		UNDEFINED,
		UI8,
		UI16,
		UI32,
		UI64,
		I8,
		I16,
		I32,
		I64,
		F8,
		F16,
		F32,
		F64,
		CHAR,
		BOOL,
		REF,
		ARRAY,
		VOID,
	};

	enum Statement_Type {
		VAR_DECLARATION,
		FUNC_CALL,
		RETURN_CALL,
		LOOP,
		IF,
	};

	enum StdLib_Class_Type {
		STDIN,
		STDOUT,
	};

	enum StdLib_Function_Type {
		READ,
		READLN,
		WRITE,
		WRITELN,
	};

	struct Type {
		std::string name;
		Builtin_Type builtinType;
	};

	struct FuncArg {
		Type mType;
		std::string mName;
	};

	struct FuncCallStatement {
		StdLib_Class_Type mClassType;
		StdLib_Function_Type mFunctionType;
		Token arg;
	};

	struct Variable {
		Type mType;
		std::string mName;
		Token mValue;
	};

	struct Range {
		int64_t mMinimum;
		int64_t mMaximum;
	};

	struct Statement;
	struct Block {
		std::vector<Statement> statements;
	};

	struct LoopStatement {
		std::optional<Variable> mIterator;
		std::optional<Range> mRange;
		Block mBody;
	};

	struct Statement {
		Statement_Type mType;
		std::string content; // TODO: Change this to proper parsing of expressions
		std::optional<FuncCallStatement> funcCall;
		std::optional<LoopStatement> loopStatement;
	};

	struct Function {
		Type mReturnType;
		std::string mName;
		std::vector<FuncArg> mArgs;
		Block mBody;
	};

	struct Literal {
		std::string mAlias;
		std::string mContent;
		uint32_t mSize;
	};

	struct Programme {
		std::vector<Function> functions;
		std::vector<Literal> literals;
		bool requires_libs = false;

		std::optional<Literal> findLiteral(const std::string& alias) {
			for (std::vector<Literal>::iterator it = literals.begin(); it != literals.end(); it++) {
				if ((*it).mAlias == alias) return (*it);
			}
			return std::nullopt;
		}
	};

	class Parser {
	public:
		Programme parse(std::vector<Token>& tokens);

	private:
		static Builtin_Type getTypeFromName(const std::string& name);
		Type getTypeFromRange(const Range& range);
		std::optional<Token> expectIdentifier(const std::string& name = "");
		std::optional<Token> expectOperator(const std::string& name = "");
		std::optional<Token> expectLiteral(const std::string& name = "");
		std::optional<Token> expectSemicolon();
		std::optional<Type> expectType(const std::string& name = "");
		std::optional<Function> expectFunction();
		std::optional<Block> expectBlock();
		std::optional<Statement> tryParseStdLibFunction();
		std::optional<Statement> tryParseLoop();

		std::vector<Token>::iterator mCurrentToken;
		std::vector<Literal> literals;
		bool requires_libs = false;
	};

} // forest::parser

#endif //FOREST_PARSER_HPP