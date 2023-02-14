#ifndef FOREST_PARSER_HPP
#define FOREST_PARSER_HPP

#include <string>
#include <vector>
#include <optional>
#include "Tokeniser.h"

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

	struct Type {
		std::string name;
		Builtin_Type builtinType;
	};

	struct FuncArg {
		Type mType;
		std::string mName;
	};

	struct Function {
		Type mReturnType;
		std::string mName;
		std::vector<FuncArg> mArgs;
		// std::vector<Statement> mBody;
	};

	class Parser {
	public:
		void parse(std::vector<Token>& tokens);

	private:
		static Builtin_Type getTypeFromName(const std::string& name);
		std::optional<Token> expectIdentifier(const std::string& name = "");
		std::optional<Token> expectOperator(const std::string& name = "");
		std::optional<Type> expectType(const std::string& name = "");
		std::optional<Function> expectFunction();

		std::vector<Token>::iterator mCurrentToken;
	};

} // forest::parser

#endif //FOREST_PARSER_HPP
