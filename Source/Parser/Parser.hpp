#ifndef FOREST_PARSER_HPP
#define FOREST_PARSER_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include "Tokeniser.hpp"
#include "Expression.hpp"

namespace forest::parser {
	enum class Builtin_Type {
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

	enum class Statement_Type {
		NOTHING,
		VAR_DECLARATION,
		VAR_ASSIGNMENT,
		FUNC_CALL,
		RETURN_CALL,
		LOOP,
		IF,
		ARRAY_INDEX,
	};

	enum class StdLib_Class_Type {
		STDIN,
		STDOUT,
	};

	enum class StdLib_Function_Type {
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
		std::vector<Expression*> mArgs;
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
		Statement_Type mType = Statement_Type::NOTHING;
		Expression* mContent{};
		std::optional<FuncCallStatement> funcCall;
		std::optional<LoopStatement> loopStatement;
		std::optional<Variable> variable;
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

		std::optional<Literal> findLiteralByAlias(const std::string& alias) const {
			for (const auto& literal : literals) {
				if (literal.mAlias == alias) return literal;
			}
			return std::nullopt;
		}

		std::optional<Literal> findLiteralByContent(const std::string& content) const {
			for (const auto& literal : literals) {
				std::string s = literal.mContent;
				if (s.rfind('\n') != std::string::npos) {
					s = s.substr(0, literal.mContent.find_last_of('\n'));
				}
				if (s == content) return literal;
			}
			return std::nullopt;
		}
	};

	class Parser {
	public:
		Programme parse(std::vector<Token>& tokens);

	public: // This was private before implementing testing. I care more about making sure the code does what it needs to do than following OOP principles.
		static Builtin_Type getTypeFromName(const std::string& name);
		Type getTypeFromRange(const Range& range);
		std::optional<Token> peekNextToken();
		std::optional<Token> expectIdentifier(const std::string& name = "");
		std::optional<Token> expectOperator(const std::string& name = "");
		std::optional<Token> expectLiteral(const std::string& name = "");
		std::optional<Token> expectSemicolon();
		std::optional<Type> expectType(const std::string& name = "");
		std::optional<Function> expectFunction();
		std::optional<Block> expectBlock();
		std::optional<Statement> expectStatement();
		std::optional<Statement> tryParseStdLibFunction();
		std::optional<Statement> tryParseLoop();
		std::optional<Statement> tryParseReturnCall();
		std::optional<Statement> tryParseVariableDeclaration();
		Expression* expectExpression(const Statement& statementContext, bool collapse = false);

		std::vector<Token>::iterator mCurrentToken;
		std::vector<Token>::iterator mTokensEnd;
		std::vector<Literal> literals;
		bool requires_libs = false;
	};

} // forest::parser

#endif //FOREST_PARSER_HPP
