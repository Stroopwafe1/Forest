#include <iostream>
#include "Parser.hpp"

namespace forest::parser {
	void Parser::parse(std::vector<Token>& tokens) {
		mCurrentToken = tokens.begin();
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
			}
		}
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
		// But we can't do that yet, so that is a todo

		for (const auto& arg : args) {
			std::cout << "FuncArg: " << arg.mName << ", " << arg.mType.name << std::endl;
		}

		return Function {type.value(), name->mText, args};
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
				// Do stuff with this
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