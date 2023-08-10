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
		ARRAY_INDEX, // Not actually a statement, but used for parsing array indexing expressions
	};

	enum class SpecialStatementType {
		NOTHING,
		DEPENDENCY,
		ASSERT,
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
		std::string mNamespace;
		std::string mClassName;
		std::string mFunctionName;
		std::vector<Expression*> mArgs;
		bool mIsExternal = false;

		bool operator ==(const FuncCallStatement& other) const {
			return mNamespace == other.mNamespace && mClassName == other.mClassName && mFunctionName == other.mFunctionName && mIsExternal == other.mIsExternal;
		}
	};

	struct Variable {
		Type mType;
		std::string mName;
		Token mValue;
	};

	struct Range {
		Expression* mMinimum;
		Expression* mMaximum;
	};

	struct Statement;
	struct Block {
		std::vector<Statement> statements;
	};

	struct LoopStatement {
		std::optional<Variable> mIterator;
		std::optional<Range> mRange;
		std::optional<Expression*> mStep;
		Block mBody;
	};

	struct IfStatement {
		std::optional<Token> mElse;
		std::optional<Block> mElseBody;
		Block mBody;
	};

	struct Statement {
		Statement_Type mType = Statement_Type::NOTHING;
		Expression* mContent{};
		std::optional<FuncCallStatement> funcCall = std::nullopt;
		std::optional<LoopStatement> loopStatement = std::nullopt;
		std::optional<Variable> variable = std::nullopt;
		std::optional<IfStatement> ifStatement = std::nullopt;
		std::vector<Statement> mSubStatements{};
	};

	struct SpecialStatement {
		SpecialStatementType mType;
		Expression* mContent{};
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
		std::vector<FuncCallStatement> externalFunctions;
		std::vector<std::string> libDependencies;
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
				if (s == content) return literal; // First try to see if it matches literally
				if (s.rfind('\n') != std::string::npos) { // Otherwise try to see if it matches without newline
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
		std::optional<SpecialStatement> expectSpecialStatement();
		std::optional<Statement> tryParseFunctionCall();
		std::optional<Statement> tryParseLoop();
		std::optional<Statement> tryParseReturnCall();
		std::optional<Statement> tryParseVariableDeclaration();
		std::optional<Statement> tryParseIfStatement();
		Expression* expectExpression(Statement& statementContext, bool collapse = false);

		std::vector<Token>::iterator mCurrentToken;
		std::vector<Token>::iterator mTokensEnd;
		std::vector<Literal> literals;
		std::vector<FuncCallStatement> externalFunctions;
		std::vector<std::string> libDependencies;
		bool requires_libs = false;
	};

} // forest::parser

#endif //FOREST_PARSER_HPP
