#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <stack>
#include "Parser.hpp"
#include "Tokeniser.hpp"

namespace forest::parser {
	Parser::Parser() {
		sizeCache = {
				{ "ui8", 1},
				{ "ui16", 2},
				{ "ui32", 4},
				{ "ui64", 8},
				{ "i8", 1},
				{ "i16", 2},
				{ "i32", 4},
				{ "i64", 8},
				{ "f8", 1},
				{ "f16", 2},
				{ "f32", 4},
				{ "f64", 8},
				{ "char", 4},
				{ "bool", 1},
		};
	}

	Programme Parser::parse(std::vector<Token>& tokens) {
		mCurrentToken = tokens.begin();
		std::vector<Function> functions;
		mTokensEnd = tokens.end();
		while (mCurrentToken != tokens.end()) {
			if (mCurrentToken->mText == "#") {
				// Parse special statement
				// #depends(c)
				// #assert(a == 3)
				// etc...
				std::optional<SpecialStatement> special = expectSpecialStatement();
				if (special.has_value()) {
					if (special.value().mType == SpecialStatementType::DEPENDENCY) {
						Token& t = special.value().mContent->mValue;
						if (t.mType == TokenType::IDENTIFIER) {
							libDependencies.push_back(t.mText);
						}
					}
				}
			} else if (mCurrentToken->mText == "struct") {
				std::optional<Struct> s = expectStruct();
				if (s.has_value()) {
					structs.insert({s.value().mName, s.value()});
					sizeCache[s.value().mName] = s.value().mSize;
				}
			}
			std::optional<Function> f = expectFunction();
			if (!f.has_value()) {
				break;
			} else {
				//std::cout << "Successfully parsed function " << f->mName << std::endl;
				functions.push_back(f.value());
			}
		}
		return Programme { functions, literals, externalFunctions, libDependencies, requires_libs };
	}

	std::optional<Token> Parser::peekNextToken() {
		if (mCurrentToken + 1 == mTokensEnd) return std::nullopt;

		return *(mCurrentToken + 1);
	}

	std::optional<Token> Parser::expectIdentifier(const std::string& name) {
		if (mCurrentToken->mType != TokenType::IDENTIFIER) return std::nullopt;
		if (!name.empty() && mCurrentToken->mText != name) return std::nullopt;

		// TODO: Also allow for namespace::type identifiers

		return *mCurrentToken++;
	}

	std::optional<Token> Parser::expectOperator(const std::string& name) {
		if (mCurrentToken->mType != TokenType::OPERATOR) return std::nullopt;
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
		while(!expectOperator(")").has_value()) {
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

		/*for (const auto& arg : args) {
			std::cout << "FuncArg: " << arg.mName << ", " << arg.mType.name << std::endl;
		}*/

		return Function {type.value(), name->mText, args, body.value()};
	}

	std::optional<Block> Parser::expectBlock() {
		std::vector<Token>::iterator saved = mCurrentToken;

		std::vector<Statement> statements;

		std::optional<Token> open_bracket = expectOperator("{");
		if (!open_bracket.has_value()) {
			std::optional<Statement> statement = expectStatement();
			if (!statement.has_value()) {
				std::cerr << "Could not parse statement at " << *mCurrentToken << std::endl;
				mCurrentToken = saved;
				return std::nullopt;
			}
			// Maybe have a debug message that mentions we parsed a simple block without brackets
			statements.push_back(statement.value());
			return Block { statements, 0 };
		}

		size_t stackMem = 0;
		while(!expectOperator("}").has_value()) {
			// Parse statements
			std::optional<Statement> statement = expectStatement();
			if (!statement.has_value()) {
				std::cerr << "Could not parse statement at " << *mCurrentToken << std::endl;
				continue;
			}

			if (statement.value().mType == Statement_Type::VAR_ASSIGNMENT || statement.value().mType == Statement_Type::VAR_DECLARATION) {
				Variable variable = statement.value().variable.value();
				stackMem += variable.mType.byteSize;
			} else if (statement.value().mType == Statement_Type::LOOP) {
				LoopStatement ls = statement.value().loopStatement.value();
				stackMem += ls.mIterator.value().mType.byteSize;
			}

			statements.push_back(statement.value());
		}

		return Block { statements, stackMem };
	}

	std::optional<Statement> Parser::expectStatement() {
		std::vector<Token>::iterator saved = mCurrentToken;

		std::optional<Statement> returnCall = tryParseReturnCall();
		if (returnCall.has_value()) {
			return returnCall.value();
		}

		std::optional<Statement> ifStatement = tryParseIfStatement();
		if (ifStatement.has_value()) {
			return ifStatement.value();
		}

		std::optional<Statement> loop = tryParseLoop();
		if (loop.has_value()) {
			return loop.value();
		}

		std::optional<Statement> functionCall = tryParseFunctionCall();
		if (functionCall.has_value()) {
			return functionCall.value();
		}

		std::optional<Statement> variable = tryParseVariableDeclaration();
		if (variable.has_value()) {
			return variable.value();
		}

		mCurrentToken = saved;
		return std::nullopt;
	}

	std::optional<SpecialStatement> Parser::expectSpecialStatement() {
		std::vector<Token>::iterator saved = mCurrentToken;
		std::optional<Token> hash = expectOperator("#");
		if (!hash.has_value()) {
			mCurrentToken = saved;
			return std::nullopt;
		}
		std::optional<Token> type = expectIdentifier();
		if (!type.has_value()) {
			mCurrentToken = saved;
			std::cerr << "Expected a function name after the # at " << *mCurrentToken << std::endl;
			return std::nullopt;
		}
		SpecialStatementType actualType = SpecialStatementType::NOTHING;
		if (type.value().mText == "depends") {
			actualType = SpecialStatementType::DEPENDENCY;
		} else if (type.value().mText == "assert") {
			actualType = SpecialStatementType::ASSERT;
		}

		std::optional<Token> paren = expectOperator("(");

		Statement s;
		s.mType = Statement_Type::FUNC_CALL;
		Expression* content = expectExpression(s);

		std::optional<Token> rparen = expectOperator(")");

		return SpecialStatement {actualType, content};
	}

	std::optional<Struct> Parser::expectStruct() {
		std::optional<Token> keyword = expectIdentifier("struct");
		if (!keyword.has_value()) return std::nullopt;
		std::vector<Token>::iterator saved = mCurrentToken;

		std::optional<Token> sname = expectIdentifier();
		if (!sname.has_value()) {
			std::cerr << "Expected struct to have a name at " << *mCurrentToken << std::endl;
			mCurrentToken = saved;
			return std::nullopt;
		}
		std::optional<Token> openingBracket = expectOperator("{");

		Struct s;
		s.mName = sname.value().mText;
		size_t offset = 0;

		while(!expectOperator("}").has_value()) {
			// Parse fields
			StructField sf;
			std::optional<Type> type = expectType();
			if (!type.has_value()) {
				std::cerr << "Expected struct field type at " << *mCurrentToken << std::endl;
				mCurrentToken = saved;
				return std::nullopt;
			}

			// Parse name(s), fields can have multiple names with the | operator
			/*
			 * struct Test {
			 * 	ui32 color|colour;
			 * 	bool val;
			 * 	bool initialised|initialized;
			 * }
			 */
			std::vector<std::string> names;
			while (!expectSemicolon().has_value()) {
				std::optional<Token> name = expectIdentifier();
				if (!name.has_value()) {
					std::cerr << "Expected struct field name at " << *mCurrentToken << std::endl;
					mCurrentToken = saved;
					return std::nullopt;
				}

				names.push_back(name.value().mText);
				if (expectSemicolon().has_value())
					break;
				else if (expectOperator("|").has_value()) {
					continue;
				} else {
					std::cerr << "Expected a ';' or '|' to separate the struct field name at " << *mCurrentToken << std::endl;
					mCurrentToken = saved;
					return std::nullopt;
				}
			}

			sf.mType = type.value();
			sf.mNames = names;
			sf.mOffset = offset;
			offset += type.value().byteSize;
			s.mFields.push_back(sf);
		}

		s.mSize = offset; // This is after all fields calculated their offset, including the last one
		return s;
	}

	std::optional<Statement> Parser::tryParseFunctionCall() {
		std::vector<Token>::iterator saved = mCurrentToken;
		Statement returnValue;
		returnValue.mType = Statement_Type::FUNC_CALL;

		// Format for function calls: [namespace::]?[class.]?[e:]?<FunctionName>(args...)
		// Optional namespace, optional class name, required function name with opening (
		// Also, function names starting with `e:` are external (really only useful for compilation step, since interpreted would need to actually link those libraries)

		FuncCallStatement fc;
		std::optional<Token> nsClassOrFunction = expectIdentifier();
		if (!nsClassOrFunction.has_value()) {
			mCurrentToken = saved;
			return std::nullopt;
		}

		std::optional<Token> ns = expectOperator("::");
		std::optional<Token> className;
		if (ns.has_value()) {
			// We know the first identifier is the namespace identifier
			// We need to parse the class identifier
			fc.mNamespace = nsClassOrFunction.value().mText;
			className = expectIdentifier();
		} else {
			className = nsClassOrFunction;
		}

		std::optional<Token> dot = expectOperator(".");
		std::optional<Token> functionName;
		if (!dot.has_value() && !ns.has_value()) {
			functionName = nsClassOrFunction;
		} else if (dot.has_value() && !className.has_value()) {
			// We got a dot after the namespace -> ::. -> Syntax error
			std::cerr << "Syntax error: Unexpected '.' after namespace operator '::' at " << *mCurrentToken << std::endl;
			mCurrentToken = saved;
			return std::nullopt;
		} else {
			fc.mClassName = className.value().mText;
			functionName = expectIdentifier();
		}

		if (functionName.has_value() && functionName.value().mText == "e") {
			std::optional<Token> externalOp = expectOperator(":");
			if (externalOp.has_value()) {
				fc.mIsExternal = true;
				functionName = expectIdentifier();
			}
			// If not, we just have a function called 'e' which is fine
		}

		fc.mFunctionName = functionName.value().mText;
		std::optional<Token> openingParenthesis = expectOperator("(");
		if (!openingParenthesis.has_value()) {
			mCurrentToken = saved;
			return std::nullopt;
		}

		std::vector<Expression*> args;
		while (!expectOperator(")").has_value()) {
			Expression* expression = expectExpression(returnValue, true);
			if (expression == nullptr) {
				std::cerr << "Expected an expression while parsing " << fc.mFunctionName << "'s argument list at " << *mCurrentToken << std::endl;
				mCurrentToken = saved;
				return std::nullopt;
			}

			args.push_back(expression);
			if (expression->mValue.mSubType == TokenSubType::STRING_LITERAL) {
				std::stringstream alias;
				alias << "str" << literals.size();

				literals.push_back(Literal{alias.str(), expression->mValue.mText, uint32_t(expression->mValue.mText.size())});
			}

			std::optional<Token> closingParenthesis = expectOperator(")");
			if (closingParenthesis.has_value())
				break;

			std::optional<Token> comma = expectOperator(",");
			if (!comma.has_value()) {
				// We didn't encounter the ')', and we didn't get a ','
				std::cerr << "Expected a ',' while parsing " << fc.mFunctionName << "'s argument list at " << *mCurrentToken << std::endl;
				mCurrentToken = saved;
				return std::nullopt;
			}
		}
		if (functionName.value().mText == "write" || functionName.value().mText == "writeln")
			requires_libs = true;
		fc.mArgs = args;

		std::optional<Token> semi = expectSemicolon();
		if (!semi.has_value()) {
			std::cerr << "Expected a closing ';' to end the statement at " << *(mCurrentToken - 1) << std::endl;
			//mCurrentToken = saved;
			// ^ Commented out because it did parse the function call correctly, it just didn't end with a semi.
			// We want it to keep giving parser errors for the rest of the code, not try to parse something else.
			return std::nullopt;
		}

		if (className.value().mText == "stdout" && functionName.value().mText == "writeln") {
			if (!literals.empty()) {
				literals.at(literals.size() - 1).mContent.append("\n");
				literals.at(literals.size() - 1).mSize += 1;
			}
		}

		returnValue.funcCall = fc;
		if (fc.mIsExternal)
			externalFunctions.push_back(fc);
		return returnValue;
	}

	std::optional<Statement> Parser::tryParseLoop() {
		std::vector<Token>::iterator saved = mCurrentToken;
		LoopStatement ls;
		// Try parse from the longest form to smallest
		std::optional<Token> loop = expectIdentifier("loop");
		Expression* content = nullptr;

		Statement s;
		s.mType = Statement_Type::LOOP;
		if (!loop.has_value()) {
			loop = expectIdentifier("until");
			if (!loop.has_value()) {
				mCurrentToken = saved;
				return std::nullopt;
			}

			Expression* condition = expectExpression(s, true);
			content = condition;
		}
		std::optional<Token> iterator = expectIdentifier();
		if (iterator.has_value()) {
			std::optional<Token> stepOp = expectOperator(":");
			if (stepOp.has_value()) {
				Expression* step = expectExpression(s, true);
				if (step != nullptr)
					ls.mStep = step;
			}

			std::optional<Token> comma = expectOperator(",");
			if (!comma.has_value()) {
				std::cerr << "Expected a ',' after the variable at " << *mCurrentToken << std::endl;
				mCurrentToken = saved;
				return std::nullopt;
			}
			// Expect a range
			Expression* min = expectExpression(s, true);
			if (min == nullptr) {
				std::cerr << "Expected a beginning of a range declaration at " << *mCurrentToken << std::endl;
				mCurrentToken = saved;
				return std::nullopt;
			}
			std::optional<Token> range = expectOperator("..");
			if (!range.has_value()) {
				std::cerr << "Expected a '..' in the range declaration at " << *mCurrentToken << std::endl;
				mCurrentToken = saved;
				return std::nullopt;
			}
			Expression* max = expectExpression(s, true);
			if (max == nullptr) {
				std::cerr << "Expected an ending of the range declaration at " << *mCurrentToken << std::endl;
				mCurrentToken = saved;
				return std::nullopt;
			}
			Range r = Range { min, max };
			ls.mRange = r;
			ls.mIterator = Variable { getTypeFromRange(r), iterator.value().mText, iterator.value() };
		}
		std::optional<Block> body = expectBlock();
		if (!body.has_value()) {
			std::cerr << "Expected a code block for the loop at " << *mCurrentToken << std::endl;
			mCurrentToken = saved;
			return std::nullopt;
		}
		ls.mBody = body.value();
		return Statement { Statement_Type::LOOP, content, std::nullopt, ls};
	}

	std::optional<Statement> Parser::tryParseReturnCall() {
		std::vector<Token>::iterator saved = mCurrentToken;
		Statement statement;

		if (!expectIdentifier("return").has_value())
			return std::nullopt;

		statement.mType = Statement_Type::RETURN_CALL;

		Expression* value = expectExpression(statement, true);
		if (value == nullptr) {
			mCurrentToken = saved;
			return std::nullopt;
		}

		statement.mContent = value;

		std::optional<Token> semi = expectSemicolon();
		if (!semi.has_value()) {
			std::cerr << "Expected a closing ';' to end the statement at " << *(mCurrentToken - 1) << std::endl;
			mCurrentToken = saved;
			return std::nullopt;
		}

		return statement;
	}

	std::optional<Statement> Parser::tryParseVariableDeclaration() {
		std::vector<Token>::iterator saved = mCurrentToken;
		Statement statement;

		std::optional<Type> type = expectType();
		if (!type.has_value()) {
			mCurrentToken = saved;
			return std::nullopt;
		}

		std::optional<Token> name = expectIdentifier();
		if (!name.has_value()) {
			std::cerr << "Expected a name for the variable declaration at " << *mCurrentToken << std::endl;
			mCurrentToken = saved;
			return std::nullopt;
		}

		std::optional<Token> semi = expectSemicolon();
		if (semi.has_value()) {
			statement.mType = Statement_Type::VAR_DECLARATION;
			statement.mContent = nullptr;
			statement.variable = Variable { type.value(), name.value().mText, name.value() };
			return statement;
		}

		std::optional<Token> assignment = expectOperator("=");
		if (!assignment.has_value()) {
			std::cerr << "Expected a '=' for the assignment of variable " << name.value().mText << " at " << *mCurrentToken << std::endl;
			mCurrentToken = saved;
			return std::nullopt;
		}

		statement.mType = Statement_Type::VAR_ASSIGNMENT;
		Expression* expression = expectExpression(statement, true);

		semi = expectSemicolon();
		if (!semi.has_value()) {
			std::cerr << "Expected a semicolon to close the variable assignment of " << name.value().mText << " at " << *mCurrentToken << std::endl;
			return std::nullopt;
		}

		statement.variable = Variable { type.value(), name.value().mText, expression->mValue };
		statement.mContent = expression;

		return statement;
	}

	std::optional<Statement> Parser::tryParseIfStatement() {
		std::vector<Token>::iterator saved = mCurrentToken;
		Statement statement;
		statement.mType = Statement_Type::IF;

		std::optional<Token> ifI = expectIdentifier("if");
		if (!ifI.has_value()) {
			mCurrentToken = saved;
			return std::nullopt;
		}

		Expression* expression = expectExpression(statement, true);
		if (expression == nullptr) {
			std::cerr << "Expected an expression after the if statement at " << *mCurrentToken << std::endl;
			return std::nullopt;
		}

		std::optional<Block> body = expectBlock();
		if (!body.has_value()) {
			std::cerr << "Expected a code block for the if statement at " << *mCurrentToken << std::endl;
			mCurrentToken = saved;
			return std::nullopt;
		}

		std::optional<Token> elseI = expectIdentifier("else");
		if (!elseI.has_value()) {
			// We have a simple if-statement `if expression { ... }`
			statement.mContent = expression;
			statement.ifStatement = IfStatement { std::nullopt, std::nullopt, body.value() };
			return statement;
		}

		std::optional<Block> elseBody = expectBlock();
		if (!elseBody.has_value()) {
			std::cerr << "Expected a code block for the else statement at " << *mCurrentToken << std::endl;
			return std::nullopt;
		}

		statement.mContent = expression;
		statement.ifStatement = IfStatement { elseI, elseBody.value(), body.value() };
		return statement;
	}

	Builtin_Type Parser::getTypeFromName(const std::string& name) {
		if (name == "ui8") return Builtin_Type::UI8;
		if (name == "ui16") return Builtin_Type::UI16;
		if (name == "ui32") return Builtin_Type::UI32;
		if (name == "ui64") return Builtin_Type::UI64;
		if (name == "i8") return Builtin_Type::I8;
		if (name == "i16") return Builtin_Type::I16;
		if (name == "i32") return Builtin_Type::I32;
		if (name == "i64") return Builtin_Type::I64;
		if (name == "f8") return Builtin_Type::F8;
		if (name == "f16") return Builtin_Type::F16;
		if (name == "f32") return Builtin_Type::F32;
		if (name == "f64") return Builtin_Type::F64;
		if (name == "char") return Builtin_Type::CHAR;
		if (name == "bool") return Builtin_Type::BOOL;
		if (name == "void") return Builtin_Type::VOID;
		if (structs.find(name) != structs.end()) return Builtin_Type::STRUCT;
		return Builtin_Type::UNDEFINED;
	}

	Type Parser::getTypeFromRange(const Range& range) {
		// TODO: We cannot know the types at compile time for some expressions
		if (range.mMinimum->mValue.mSubType != TokenSubType::INTEGER_LITERAL)
			return Type {"undefined", Builtin_Type::UNDEFINED};
		if (range.mMaximum->mValue.mSubType != TokenSubType::INTEGER_LITERAL)
			return Type {"undefined", Builtin_Type::UNDEFINED};

		long range_min = std::stol(range.mMinimum->mValue.mText);
		long range_max = std::stol(range.mMaximum->mValue.mText);
		int64_t min = range_min < range_max ? range_min : range_max;
		int64_t max = range_max > range_min ? range_max : range_min;

		if (min < 0) {
			// Has to be signed
			if (min >= -128 && max <= 127) {
				return Type { "i8", Builtin_Type::I8, {}, 1 };
			} else if (min >= -32768 && max <= 32767) {
				return Type { "i16", Builtin_Type::I16, {}, 2 };
			} else if (min >= -2147483648 && max <= 2147483647) {
				return Type { "i32", Builtin_Type::I32, {}, 4 };
			} else {
				return Type { "i64", Builtin_Type::I64, {}, 8 };
			}
		} else {
			if (max <= 255) {
				return Type { "ui8", Builtin_Type::UI8, {}, 1 };
			} else if (max <= 65535) {
				return Type { "ui16", Builtin_Type::UI16, {}, 2 };
			} else if (max <= 4294967295) {
				return Type { "ui32", Builtin_Type::UI32, {}, 4 };
			} else {
				return Type { "ui64", Builtin_Type::UI64, {}, 8 };
			}
		}
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
				size_t size = 0;
				const auto& found = sizeCache.find(id->mText);
				if (found != sizeCache.end())
					size = found->second;
				return Type {id->mText, getTypeFromName(id->mText), {}, size};
			} else {
				// Child element
				std::optional<Type> childType = expectType();
				if (!childType.has_value()) {
					std::cerr << "Expected a type within the <...> of type '" << id->mText << "' at " << *mCurrentToken << std::endl;
					mCurrentToken = saved;
					return std::nullopt;
				}
				std::vector<Type> types;
				types.push_back(childType.value());
				if (id->mText == "array") {
					std::optional<Token> semi = expectSemicolon();
					size_t len = 1;
					if (semi.has_value()) {
						std::optional<Token> length = expectLiteral();
						if (!length.has_value())
							std::cerr << "Expected a length after the ';' in the generic array type at " << *mCurrentToken << std::endl;
						else {
							len = stol(length.value().mText);
						}
					}
					return Type{"array<>", Builtin_Type::ARRAY, types, len * childType.value().byteSize};
				} else if (id->mText == "ref") {
					return Type{"ref<>", Builtin_Type::REF, types, 8};
				} else {
					// TODO: This byteSize value is not correct
					// Basically, we have a generic class here, so we should fetch its size + size of generic argument * how many times it's used, or if just a reference, 8
					/* class Test<T> {
					 * 	T val // This would add the size of generic argument
					 * 	ref<T> val2 // This would only reference size, which is 8; size of T doesn't matter
					 * 	T[] val3 // This would add size of generic argument * size of array
					 * }
					 */
					return Type{id->mText + "<>", Builtin_Type::UNDEFINED, types, 0};
				}
			}
		} else {
			size_t len = 1;
			std::optional<Token> length = expectLiteral();
			if (length.has_value())
				len = stol(length.value().mText);

			std::optional<Token> closingBracket = expectOperator("]");
			if (!closingBracket.has_value()) {
				std::cerr << "Expected a ']' to close the opening '[' at " << openingBracket.value() << " in type declaration." << std::endl;
				mCurrentToken = saved;
				return std::nullopt;
			}

			std::vector<Type> types;
			Type t = Type {id->mText, getTypeFromName(id->mText), {}, 0};

			size_t size = 8;
			const auto& found = sizeCache.find(id->mText);
			if (found != sizeCache.end()) {
				size = found->second;
			}
			t.byteSize = size;

			types.emplace_back(t);
			return Type {"array[]", Builtin_Type::ARRAY, types, len * size};
		}

		return Type {id->mText, getTypeFromName(id->mText)};
	}

	Expression* Parser::expectExpression(Statement& statementContext, bool collapse) {
		std::vector<Token>::iterator saved = mCurrentToken;
		// While no semicolon for variable assignment or return call, parse
		// While no ')' for function calls or if-statements, parse
		// While no ']' for array indexing, parse
		// While no '{' for range statement, parse
		std::stack<char> parenStack;
		std::vector<Expression*> nodes;

		// While the next token exists and is not a closing character for the current statement context
		while (mCurrentToken != mTokensEnd && !((
					((mCurrentToken->mText == ")" || mCurrentToken->mText == "," || mCurrentToken->mText == "{") && (statementContext.mType == Statement_Type::FUNC_CALL || statementContext.mType == Statement_Type::IF)) ||
					(mCurrentToken->mText == "]" && (statementContext.mType == Statement_Type::ARRAY_INDEX)) ||
					((mCurrentToken->mText == "{" || mCurrentToken->mText == ".." || mCurrentToken->mText == ",") && (statementContext.mType == Statement_Type::LOOP)) ||
					(mCurrentToken->mType == TokenType::SEMICOLON)
				) && parenStack.empty() )
			) {
			if (mCurrentToken->mText == "(") {
				parenStack.push('(');
				expectOperator("(");
			} else if (mCurrentToken->mText == ")") {
				expectOperator(")");
				// Pop parenStack
				// Pop last 3 nodes
				// Put middle node as root
				// Push back root node into vector
				parenStack.pop();
				Expression* right = nodes.back();
				nodes.pop_back();
				Expression* op = nodes.back();
				nodes.pop_back();
				Expression* left = nodes.back();
				nodes.pop_back();

				if (op->mValue.mType != TokenType::OPERATOR) {
					std::cerr << "Expected an operator while parsing the expression at " << op->mValue << " but got '" << op->mValue.mText << "' instead." << std::endl;
					mCurrentToken = saved;
					return nullptr;
				}

				op->mLeft = left;
				op->mRight = right;
				nodes.push_back(op);
			} else if (mCurrentToken->mType == TokenType::IDENTIFIER) {
				std::optional<Token> nextToken = peekNextToken();
				std::optional<Token> identifier = expectIdentifier();
				if (nextToken.has_value() && nextToken.value().mText == "[") {
					std::optional<Token> arrayIndex = expectOperator("[");
					Statement newStatement;
					newStatement.mType = Statement_Type::ARRAY_INDEX;

					// Array indexing expression is the following tree
					//            [
					//         /     \
					//        id       index of array
					Expression* node = new Expression;
					node->mValue = arrayIndex.value();
					Expression* left = new Expression;
					left->mValue = identifier.value();
					Expression* right = expectExpression(newStatement);
					expectOperator("]"); // We discard this value because we don't need it
					node->mLeft = left;
					node->mRight = right;
					nodes.push_back(node);
				} else if (nextToken.has_value() && nextToken.value().mText == "(") {
					// Can only be function call, would be regular operator if this is meant to be a variable
					// ui8 foo = bar(
					// TODO: Make function calls more generic than standard library function
					std::optional<Token> funcCall = expectOperator("(");
					Statement newStatement;
					newStatement.mType = Statement_Type::FUNC_CALL;
					Expression* node = new Expression;
					node->mValue = funcCall.value();
					Expression* left = new Expression;
					left->mValue = identifier.value();
					Expression* right = expectExpression(newStatement);
					expectOperator(")"); // We discard this value because we don't need it
					node->mLeft = left;
					node->mRight = right;
					nodes.push_back(node);
				} else {
					Expression* node = new Expression;
					node->mValue = identifier.value();
					nodes.push_back(node);
				}
			} else if (mCurrentToken->mType == TokenType::OPERATOR) {
				std::optional<Token> op = expectOperator();
				Expression* node = new Expression;
				node->mValue = op.value();
				nodes.push_back(node);
			} else if (mCurrentToken->mType == TokenType::LITERAL) {
				std::optional<Token> literal = expectLiteral();
				Expression* node = new Expression;
				node->mValue = literal.value();
				nodes.push_back(node);
			}

			// E.g. (4 - (3 + 1)) or ((4 - 3) + 1)
		} // End of while
		if (nodes.size() == 3) {
			Expression* right = nodes.back();
			nodes.pop_back();
			Expression* op = nodes.back();
			nodes.pop_back();
			Expression* left = nodes.back();
			nodes.pop_back();

			if (op->mValue.mType != TokenType::OPERATOR) {
				std::cerr << "Expected an operator while parsing the expression at " << op->mValue << " but got '" << op->mValue.mText << "' instead." << std::endl;
				mCurrentToken = saved;
				return nullptr;
			}

			op->mLeft = left;
			op->mRight = right;
			nodes.push_back(op);
		}

		if (nodes.empty()) {
			std::cerr << "Zero nodes found while parsing expression at " << *mCurrentToken << std::endl;
			mCurrentToken = saved;
			return nullptr;
		} else if (nodes.size() > 1) {
			std::cerr << "Unexpected amount of nodes while parsing expression. Found " << nodes.size() << " but expected one. Full list:" << std::endl;
			for (auto node : nodes) {
				std::cerr << "Unexpected node at " << (*node).mValue << std::endl;
			}
			mCurrentToken = saved;
			return nullptr;
		}

		if (collapse)
			nodes[0]->Collapse();

		return nodes[0];
	}


} // forest::parser
