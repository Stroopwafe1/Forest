#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <stack>
#include "Parser.hpp"
#include "Tokeniser.hpp"

namespace forest::parser {
	Programme Parser::parse(std::vector<Token>& tokens) {
		mCurrentToken = tokens.begin();
		std::vector<Function> functions;
		mTokensEnd = tokens.end();
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
				//std::cout << "Successfully parsed function " << f->mName << std::endl;
				functions.push_back(f.value());
			}
		}
		return Programme { functions, literals, externalFunctions, requires_libs };
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

		std::optional<Token> open_bracket = expectOperator("{");
		if (!open_bracket.has_value()) {
			std::cerr << "Expected a '{' while parsing a scoped block at " << *mCurrentToken << std::endl;
			mCurrentToken = saved;
			return std::nullopt;
		}

		std::vector<Statement> statements;

		while(!expectOperator("}").has_value()) {
			// Parse statements
			std::optional<Statement> statement = expectStatement();
			if (!statement.has_value()) {
				std::cerr << "Could not parse statement at " << *mCurrentToken << std::endl;
			}

			statements.push_back(statement.value());
		}

		return Block { statements };
	}

	std::optional<Statement> Parser::expectStatement() {
		std::vector<Token>::iterator saved = mCurrentToken;

		std::optional<Statement> returnCall = tryParseReturnCall();
		if (returnCall.has_value()) {
			return returnCall.value();
		}

		std::optional<Statement> functionCall = tryParseFunctionCall();
		if (functionCall.has_value()) {
			return functionCall.value();
		}

		std::optional<Statement> loop = tryParseLoop();
		if (loop.has_value()) {
			return loop.value();
		}

		std::optional<Statement> variable = tryParseVariableDeclaration();
		if (variable.has_value()) {
			return variable.value();
		}

		mCurrentToken = saved;
		return std::nullopt;
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
		if (!loop.has_value()) {
			loop = expectIdentifier("until");
			if (!loop.has_value()) {
				mCurrentToken = saved;
				return std::nullopt;
			}
			Statement s;
			s.mType = Statement_Type::LOOP;
			Expression* condition = expectExpression(s, true);
			content = condition;
		}
		std::optional<Token> iterator = expectIdentifier();
		if (iterator.has_value()) {
			std::optional<Token> comma = expectOperator(",");
			if (!comma.has_value()) {
				std::cerr << "Expected a ',' after the variable at " << *mCurrentToken << std::endl;
				mCurrentToken = saved;
				return std::nullopt;
			}
			// Expect a range
			Statement s;
			s.mType = Statement_Type::LOOP;
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
			statement.loopStatement = std::nullopt;
			statement.funcCall = std::nullopt;
			statement.variable = Variable { type.value(), name.value().mText, name.value() };
			return statement;
		}

		std::optional<Token> assignment = expectOperator("=");
		if (!name.has_value()) {
			std::cerr << "Expected a '=' for the assignment of variable " << name.value().mText << " at " << *mCurrentToken << std::endl;
			mCurrentToken = saved;
			return std::nullopt;
		}

		statement.mType = Statement_Type::VAR_ASSIGNMENT;
		Expression* expression = expectExpression(statement, true);
		statement.loopStatement = std::nullopt;
		statement.funcCall = std::nullopt;
		statement.variable = Variable { type.value(), name.value().mText, expression->mValue };
		statement.mContent = expression;

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
				return Type { "i8", Builtin_Type::I8 };
			} else if (min >= -32768 && max <= 32767) {
				return Type { "i16", Builtin_Type::I16 };
			} else if (min >= -2147483648 && max <= 2147483647) {
				return Type { "i32", Builtin_Type::I32 };
			} else {
				return Type { "i64", Builtin_Type::I64 };
			}
		} else {
			if (max <= 255) {
				return Type { "ui8", Builtin_Type::UI8 };
			} else if (max <= 65535) {
				return Type { "ui16", Builtin_Type::UI16 };
			} else if (max <= 4294967295) {
				return Type { "ui32", Builtin_Type::UI32 };
			} else {
				return Type { "ui64", Builtin_Type::UI64 };
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
			return Type {id->mText + "[]", Builtin_Type::ARRAY};
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
					((mCurrentToken->mText == ")" || mCurrentToken->mText == ",") && (statementContext.mType == Statement_Type::FUNC_CALL || statementContext.mType == Statement_Type::IF)) ||
					(mCurrentToken->mText == "]" && (statementContext.mType == Statement_Type::ARRAY_INDEX)) ||
					((mCurrentToken->mText == "{" || mCurrentToken->mText == "..") && (statementContext.mType == Statement_Type::LOOP)) ||
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
