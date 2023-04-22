#include <iostream>
#include <optional>
#include <sstream>
#include "Parser.hpp"
#include "Tokeniser.h"

namespace forest::parser {
	Programme Parser::parse(std::vector<Token>& tokens) {
		mCurrentToken = tokens.begin();
		std::vector<Function> functions;
		while (mCurrentToken != tokens.end()) {
			/*std::optional<Token> id = expectIdentifier();
			if (!id.has_value()) {
				std::cerr << "Expected identifier at: " << *mCurrentToken << ". But found a " << mCurrentToken->getType() << " instead with value '" << mCurrentToken->mText << "'." << std::endl;
				mCurrentToken++;
			} else {
				std::cout << "Successfully parsed identifier at " << id.value() << " with value: " << id.value().mText << std::endl;
			}*/
			std::optional<Function> f = expectFunction();
			if (!f.has_value()) {
				break;
			} else {
				std::cout << "Successfully parsed function " << f->mName << std::endl;
				functions.push_back(f.value());
			}
		}
		return Programme { functions, literals };
	}

	std::optional<Token> Parser::expectIdentifier(const std::string& name) {
		if (mCurrentToken->mType != IDENTIFIER) return std::nullopt;
		if (!name.empty() && mCurrentToken->mText != name) return std::nullopt;

		// TODO: Also allow for namespace::type identifiers

		return *mCurrentToken++;
	}

	std::optional<Token> Parser::expectOperator(const std::string& name) {
		if (mCurrentToken->mType != OPERATOR) return std::nullopt;
		if (!name.empty() && mCurrentToken->mText != name) return std::nullopt;

		return *mCurrentToken++;
	}

	std::optional<Token> Parser::expectLiteral(const std::string& name) {
		if (mCurrentToken->mType != TokenType::LITERAL) return std::nullopt;
		if (!name.empty() && mCurrentToken->mText != name) return std::nullopt;

		return *mCurrentToken++;
	}

	std::optional<Token> Parser::expectSemicolon() {
		if (mCurrentToken->mType != TokenType::SEMICOLON) return std::nullopt;

		return *mCurrentToken++;
	}

	std::optional<Function> Parser::expectFunction() {
		std::vector<Token>::iterator saved = mCurrentToken;
		std::optional<Type> type = expectType();
		if (!type.has_value()) {
			mCurrentToken = saved;
			return std::nullopt;
		}
		std::optional<Token> name = expectIdentifier();
		if (!name.has_value()) {
			mCurrentToken = saved;
			return std::nullopt;
		}
		std::optional<Token> openingParenthesis = expectOperator("(");
		if (!openingParenthesis.has_value()) {
			mCurrentToken = saved;
			return std::nullopt;
		}

		// Parse parameter list
		std::vector<FuncArg> args;
		while(expectOperator(")") == std::nullopt) {
			std::optional<Type> pType = expectType();
			if (!pType.has_value()) {
				std::cerr << "Expected a type declaration in " << name.value().mText << "'s parameter list at " << *mCurrentToken << std::endl;
				mCurrentToken = saved;
				return std::nullopt;
			}
			std::optional<Token> pName = expectIdentifier();
			if (!pName.has_value()) {
				std::cerr << "Expected a parameter name in " << name.value().mText << "'s parameter list at " << *mCurrentToken << std::endl;
				mCurrentToken = saved;
				return std::nullopt;
			}

			args.push_back(FuncArg {pType.value(), pName->mText});

			std::optional<Token> closingParenthesis = expectOperator(")");
			if (closingParenthesis.has_value())
				break;

			std::optional<Token> comma = expectOperator(",");
			if (!comma.has_value()) {
				// We didn't encounter the ')', and we didn't get a ','
				std::cerr << "Expected a ',' in " << name.value().mText << "'s parameter list at " << *mCurrentToken << std::endl;
				mCurrentToken = saved;
				return std::nullopt;
			}
		}

		// Parse function body, which is the same as parsing a scoped block.
		// We have a start, but it's nowhere near accurate
		std::optional<Block> body = expectBlock();
		if (!body.has_value()) {
			mCurrentToken = saved;
			return std::nullopt;
		}
		std::vector<Statement> statements = body.value().statements;

		if (type.value().builtinType != Builtin_Type::VOID) {
			bool found = false;
			for (std::vector<Statement>::iterator it = statements.begin(); it != statements.end(); it++) {
				if ((*it).mType == Statement_Type::RETURN_CALL) {
					found = true;
				}
			}
			if (!found) {
				std::cerr << "Expected a return statement of type " << type.value().name << " in " << name.value().mText << "'s function body at " << *(mCurrentToken - 1) << std::endl;
				mCurrentToken = saved;
				return std::nullopt;
			}
		}

		for (const auto& arg : args) {
			std::cout << "FuncArg: " << arg.mName << ", " << arg.mType.name << std::endl;
		}

		return Function {type.value(), name->mText, args, body.value()};
	}

	std::optional<Block> Parser::expectBlock() {
		std::vector<Token>::iterator saved = mCurrentToken;

		std::optional<Token> open_bracket = expectOperator("{");
		if (!open_bracket.has_value()) {
			std::cerr << "Expected a '{' while parsing a scoped block at " << *mCurrentToken << std::endl;
			mCurrentToken = saved;
			return std::nullopt;
		}

		std::vector<Statement> statements;

		while(expectOperator("}") == std::nullopt) {
			// Parse statements
			Statement statement;

			// We only allow return statements and a couple function calls for now

			if (expectIdentifier("return").has_value()) {
				std::optional<Token> value = expectLiteral();
				statement.mType = Statement_Type::RETURN_CALL;
				statement.content = value.value().mText;
			}

			// TODO: This can cause a return statement to be overwritten. But I can't be bothered to fix it up right now
			std::optional<Statement> stdlib_call = tryParseStdLibFunction();
			if (stdlib_call.has_value()) {
				statement = stdlib_call.value();
			}

			std::optional<Token> semi = expectSemicolon();
			if (!semi.has_value()) {
				std::cerr << "Expected a closing ';' to end the statement at " << *mCurrentToken << std::endl;
				mCurrentToken = saved;
				return std::nullopt;
			}
			statements.push_back(statement);
		}

		return Block { statements };
	}

	std::optional<Statement> Parser::tryParseStdLibFunction() {
		std::vector<Token>::iterator saved = mCurrentToken;
		Statement returnValue;
		FuncCallStatement fc;
		std::optional<Token> className = expectIdentifier();
		if (!className.has_value()) {
			mCurrentToken = saved;
			return std::nullopt;
		}
		std::optional<Token> dot = expectOperator(".");
		if (!dot.has_value()) {
			mCurrentToken = saved;
			return std::nullopt;
		}
		std::optional<Token> functionName = expectIdentifier();
		if (!functionName.has_value()) {
			mCurrentToken = saved;
			return std::nullopt;
		}
		std::optional<Token> openingParenthesis = expectOperator("(");
		if (!openingParenthesis.has_value()) {
			mCurrentToken = saved;
			return std::nullopt;
		}
		// Arguments are more dynamic, but TODO: We only accept string literals
		std::optional<Token> arg = expectLiteral();
		if (!arg.has_value()) {
			mCurrentToken = saved;
			return std::nullopt;
		}

		std::stringstream alias;
		alias << "str" << literals.size();

		literals.push_back(Literal { alias.str(), arg.value().mText, uint32_t(arg.value().mText.size()) });
		std::optional<Token> closingParenthesis = expectOperator(")");
		if (!closingParenthesis.has_value()) {
			mCurrentToken = saved;
			return std::nullopt;
		}

		if (className.value().mText == "stdin") {
			fc.mClassType = StdLib_Class_Type::STDIN;
		} else {
			fc.mClassType = StdLib_Class_Type::STDOUT;
		}

		if (functionName.value().mText == "read") {
			fc.mFunctionType = StdLib_Function_Type::READ;
		} else if (functionName.value().mText == "readln") {
			fc.mFunctionType = StdLib_Function_Type::READLN;
		} else if (functionName.value().mText == "write") {
			fc.mFunctionType = StdLib_Function_Type::WRITE;
		} else if (functionName.value().mText == "writeln") {
			fc.mFunctionType = StdLib_Function_Type::WRITELN;
			literals.at(literals.size() - 1).mContent.append("\n");
			literals.at(literals.size() - 1).mSize += 1;
		}

		returnValue.content = alias.str();
		returnValue.mType = Statement_Type::FUNC_CALL;
		returnValue.funcCall = fc;
		return returnValue;
	}

	Builtin_Type Parser::getTypeFromName(const std::string& name) {
		if (name == "ui8") return UI8;
		if (name == "ui16") return UI16;
		if (name == "ui32") return UI32;
		if (name == "ui64") return UI64;
		if (name == "i8") return I8;
		if (name == "i16") return I16;
		if (name == "i32") return I32;
		if (name == "i64") return I64;
		if (name == "f8") return F8;
		if (name == "f16") return F16;
		if (name == "f32") return F32;
		if (name == "f64") return F64;
		if (name == "char") return CHAR;
		if (name == "bool") return BOOL;
		if (name == "void") return VOID;
		return UNDEFINED;
	}

	std::optional<Type> Parser::expectType(const std::string& name) {
		if (!name.empty() && mCurrentToken->mText != name) return std::nullopt;
		std::vector<Token>::iterator saved = mCurrentToken;

		// 3 options:
		// Identifier
		// Identifier[]
		// Identifier<Type>
		std::optional<Token> id = expectIdentifier();
		if (!id.has_value()) return std::nullopt;

		std::optional<Token> openingBracket = expectOperator("[");
		if (!openingBracket.has_value()) {
			std::optional<Token> openingSharp = expectOperator("<");
			if (!openingSharp.has_value()) {
				return Type {id->mText, getTypeFromName(id->mText)};
			} else {
				// Child element
				std::optional<Type> childType = expectType();
				// TODO: Do stuff with this
			}
		} else {
			std::optional<Token> closingBracket = expectOperator("]");
			if (!closingBracket.has_value()) {
				std::cerr << "Expected a ']' to close the opening '[' at " << openingBracket.value() << " in type declaration." << std::endl;
				mCurrentToken = saved;
				return std::nullopt;
			}
			return Type {id->mText + "[]", ARRAY};
		}

		return Type {id->mText, getTypeFromName(id->mText)};
	}
} // forest::parser
