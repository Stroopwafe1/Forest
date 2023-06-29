#include <cctype>
#include <stdexcept>
#include <iostream>
#include "Tokeniser.hpp"

namespace forest::parser {
	std::vector<Token> Tokeniser::parse(const std::string &inProgram, const std::string& fileName) {
		std::vector<Token> tokens;
		Token currentToken;
		currentToken.file = fileName;
		
		int start = 0;
		int end = 0;
		for (size_t i = 0; i < inProgram.size(); i++) {
			char currChar = inProgram[i];
			start++;
			end++;
			Token lastToken {};
			if (!tokens.empty())
				lastToken = tokens.back();

			if (currentToken.mType == TokenType::STRING_ESCAPE_SEQUENCE || currentToken.mType == TokenType::CHAR_ESCAPE_SEQUENCE) {
				switch (currChar) {
					case 'n':
						currentToken.mText.append(1, '\n');
						break;
					case 't':
						currentToken.mText.append(1, '\t');
						break;
					case 'r':
						currentToken.mText.append(1, '\r');
						break;
					case '\\':
						currentToken.mText.append(1, '\\');
						break;
					default:
						throw std::runtime_error(std::string("Unknown escape sequence: \\") + currChar +
												 " in string at " + std::to_string(currentToken.mLineNumber) + ":" +
												 std::to_string(currentToken.mEndOffset));
				}
				currentToken.mType = TokenType::LITERAL;
				if (currentToken.mType == TokenType::STRING_ESCAPE_SEQUENCE)
					currentToken.mSubType = TokenSubType::STRING_LITERAL;
				else if (currentToken.mType == TokenType::CHAR_ESCAPE_SEQUENCE)
					currentToken.mSubType = TokenSubType::CHAR_LITERAL;
				currentToken.mEndOffset = end + 1;
				continue;
			} else if (currentToken.mSubType == TokenSubType::STRING_LITERAL && currChar != '\\' && currChar != '"') {
				currentToken.mText.append(1, currChar);
				continue;
			} else if (currentToken.mSubType == TokenSubType::CHAR_LITERAL && currChar != '\\' && currChar != '\'') {
				currentToken.mText.append(1, currChar);
				continue;
			} else if (currentToken.mType == TokenType::POTENTIAL_COMMENT && !(currChar == '/' || currChar == '*')) {
				currentToken.mType = TokenType::OPERATOR;
				currentToken.mSubType = TokenSubType::NOTHING;
				endToken(currentToken, tokens);
			} else if (currentToken.mType == TokenType::MULTILINE_COMMENT && currChar != '*' && inProgram[i+1] != '/') {
				currentToken.mText.append(1, currChar);
				continue;
			} else if (currentToken.mType == TokenType::MULTILINE_COMMENT && currChar == '*' && inProgram[i+1] == '/') {
				endToken(currentToken, tokens);
				continue;
			} else if (currentToken.mType == TokenType::SINGLELINE_COMMENT && currChar != '\n') {
				currentToken.mText.append(1, currChar);
				continue;
			} else if (currentToken.mType == TokenType::POTENTIAL_NEGATIVE_NUMBER && !isdigit(currChar)) {
				currentToken.mType = TokenType::OPERATOR;
				currentToken.mSubType = TokenSubType::NOTHING;
				endToken(currentToken, tokens);
			}

			switch (currChar) {
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					if (currentToken.mType == TokenType::NOTHING) {
						currentToken.mType = TokenType::LITERAL;
						currentToken.mSubType = TokenSubType::INTEGER_LITERAL;
						currentToken.mStartOffset = start;
						currentToken.mEndOffset = end + 1;
						currentToken.mText.erase();
						currentToken.mText.append(1, currChar);
					} else if (currentToken.mType == TokenType::POTENTIAL_NEGATIVE_NUMBER) {
						currentToken.mType = TokenType::LITERAL;
						currentToken.mSubType = TokenSubType::INTEGER_LITERAL;
						currentToken.mEndOffset = end + 1;
						currentToken.mText.append(1, currChar);
					} else if (currentToken.mType == TokenType::OPERATOR) {
						endToken(currentToken, tokens);
						currentToken.mType = TokenType::LITERAL;
						currentToken.mSubType = TokenSubType::INTEGER_LITERAL;
						currentToken.mEndOffset = end + 1;
						currentToken.mText.append(1, currChar);
					} else {
						currentToken.mEndOffset = end + 1;
						currentToken.mText.append(1, currChar);
					}
					break;

				case '.':
					if ((currentToken.mType == TokenType::NOTHING || currentToken.mType == TokenType::IDENTIFIER || currentToken.mType == TokenType::OPERATOR) && lastToken.mSubType != TokenSubType::DOT) {
						endToken(currentToken, tokens);
						currentToken.mType = TokenType::OPERATOR;
						currentToken.mSubType = TokenSubType::DOT;
						currentToken.mStartOffset = start;
						currentToken.mEndOffset = end + 1;
						currentToken.mText.append(1, currChar);
						endToken(currentToken, tokens);
					} else if (currentToken.mSubType == TokenSubType::FLOAT_LITERAL) {// A *second* decimal point -> range operator
						// Split into 2 tokens, integer literal and range op
						currentToken.mText.erase(currentToken.mText.size() - 1);
						currentToken.mSubType = TokenSubType::INTEGER_LITERAL;
						currentToken.mEndOffset = start - 2;
						endToken(currentToken, tokens);

						currentToken.mType = TokenType::OPERATOR;
						currentToken.mSubType = TokenSubType::RANGE;
						currentToken.mStartOffset = start;
						currentToken.mEndOffset = end + 1;
						currentToken.mText.append(2, currChar);
						endToken(currentToken, tokens);
					} else if (lastToken.mSubType == TokenSubType::DOT) { // A 2nd dot -> range operator
						currentToken.mText.append(1, currChar);
						currentToken.mType = TokenType::OPERATOR;
						currentToken.mSubType = TokenSubType::RANGE;
						currentToken.mStartOffset = start;
						currentToken.mEndOffset = end + 1;
						endToken(currentToken, tokens);
					} else if (currentToken.mSubType == TokenSubType::INTEGER_LITERAL) {// Integers can't contain dots, so this must be a decimal number.
						currentToken.mSubType = TokenSubType::FLOAT_LITERAL;
						currentToken.mEndOffset = end + 1;
						currentToken.mText.append(1, currChar);
					} else if (currentToken.mType != TokenType::NOTHING) {
						currentToken.mEndOffset = end + 1;
						currentToken.mText.append(1, currChar);
					}
					break;

				case '/':
					if (currentToken.mSubType == TokenSubType::STRING_LITERAL) {
						currentToken.mEndOffset = end + 1;
						currentToken.mText.append(1, currChar);
					} else if (currentToken.mType == TokenType::POTENTIAL_COMMENT) {
						currentToken.mType = TokenType::SINGLELINE_COMMENT;
						currentToken.mSubType = TokenSubType::NOTHING;
						currentToken.mText.erase();
					} else {
						endToken(currentToken, tokens);
						currentToken.mType = TokenType::POTENTIAL_COMMENT;
						currentToken.mSubType = TokenSubType::NOTHING;
						currentToken.mStartOffset = start;
						currentToken.mEndOffset = end + 1;
						currentToken.mText.append(1, currChar);
					}

					break;
				case '-':
					endToken(currentToken, tokens);
					currentToken.mType = TokenType::POTENTIAL_NEGATIVE_NUMBER;
					currentToken.mSubType = TokenSubType::NOTHING;
					currentToken.mStartOffset = start;
					currentToken.mEndOffset = end + 1;
					currentToken.mText.append(1, currChar);
					break;

				case '{':
				case '}':
				case '(':
				case ')':
				case '[':
				case ']':
				case ',':
				case '!':
				case '#':
				case '@':
				case '<':
				case '>':
				case '+':
				case '%':
					endToken(currentToken, tokens);
					currentToken.mType = TokenType::OPERATOR;
					currentToken.mSubType = TokenSubType::NOTHING;
					currentToken.mStartOffset = start;
					currentToken.mEndOffset = end + 1;
					currentToken.mText.erase();
					currentToken.mText.append(1, currChar);
					break;
				case '=':
					if (currentToken.mType == TokenType::OPERATOR &&
					(currentToken.mText == "-" ||
					currentToken.mText == "=" ||
					currentToken.mText == "+" ||
					currentToken.mText == "*" ||
					currentToken.mText == "/" ||
					currentToken.mText == "%" ||
					currentToken.mText == "<" ||
					currentToken.mText == ">" ||
					currentToken.mText == "|" ||
					currentToken.mText == "&" ||
					currentToken.mText == "!")) {
						currentToken.mText.append(1, currChar);
						currentToken.mEndOffset++;
						currentToken.mSubType = TokenSubType::NAMESPACE;
						endToken(currentToken, tokens);
						break;
					}
					endToken(currentToken, tokens);
					currentToken.mType = TokenType::OPERATOR;
					currentToken.mSubType = TokenSubType::NOTHING;
					currentToken.mStartOffset = start;
					currentToken.mEndOffset = end + 1;
					currentToken.mText.erase();
					currentToken.mText.append(1, currChar);
					break;
				case ':':
					if (currentToken.mType == TokenType::OPERATOR && currentToken.mText == ":") {
						currentToken.mText.append(1, currChar);
						currentToken.mEndOffset++;
						currentToken.mSubType = TokenSubType::NAMESPACE;
						endToken(currentToken, tokens);
						break;
					}
					endToken(currentToken, tokens);
					currentToken.mType = TokenType::OPERATOR;
					currentToken.mSubType = TokenSubType::NOTHING;
					currentToken.mStartOffset = start;
					currentToken.mEndOffset = end + 1;
					currentToken.mText.erase();
					currentToken.mText.append(1, currChar);
					break;
				case '|':
					if (currentToken.mType == TokenType::OPERATOR && currentToken.mText == "|") {
						currentToken.mText.append(1, currChar);
						currentToken.mEndOffset++;
						currentToken.mSubType = TokenSubType::NAMESPACE;
						endToken(currentToken, tokens);
						break;
					}
					endToken(currentToken, tokens);
					currentToken.mType = TokenType::OPERATOR;
					currentToken.mSubType = TokenSubType::NOTHING;
					currentToken.mStartOffset = start;
					currentToken.mEndOffset = end + 1;
					currentToken.mText.erase();
					currentToken.mText.append(1, currChar);
					break;
				case '&':
					if (currentToken.mType == TokenType::OPERATOR && currentToken.mText == "&") {
						currentToken.mText.append(1, currChar);
						currentToken.mEndOffset++;
						currentToken.mSubType = TokenSubType::NAMESPACE;
						endToken(currentToken, tokens);
						break;
					}
					endToken(currentToken, tokens);
					currentToken.mType = TokenType::OPERATOR;
					currentToken.mSubType = TokenSubType::NOTHING;
					currentToken.mStartOffset = start;
					currentToken.mEndOffset = end + 1;
					currentToken.mText.erase();
					currentToken.mText.append(1, currChar);
					break;
				case '*':
					if (currentToken.mType == TokenType::OPERATOR && currentToken.mText == "*") {
						currentToken.mText.append(1, currChar);
						currentToken.mEndOffset++;
						currentToken.mSubType = TokenSubType::NAMESPACE;
						endToken(currentToken, tokens);
						break;
					} else if (currentToken.mType == TokenType::POTENTIAL_COMMENT) {
						currentToken.mType = TokenType::MULTILINE_COMMENT;
						currentToken.mSubType = TokenSubType::NOTHING;
						currentToken.mText.erase();
						break;
					}
					endToken(currentToken, tokens);
					currentToken.mType = TokenType::OPERATOR;
					currentToken.mSubType = TokenSubType::NOTHING;
					currentToken.mStartOffset = start;
					currentToken.mEndOffset = end + 1;
					currentToken.mText.erase();
					currentToken.mText.append(1, currChar);
					break;
				case '"':
					if (currentToken.mSubType == TokenSubType::STRING_LITERAL) {
						currentToken.mEndOffset = end + 1;
						endToken(currentToken, tokens);
					} else {
						if (currentToken.mType != TokenType::NOTHING) {
							endToken(currentToken, tokens);
						}
						currentToken.mType = TokenType::LITERAL;
						currentToken.mSubType = TokenSubType::STRING_LITERAL;
						currentToken.mText.erase();
						currentToken.mStartOffset = start;
						currentToken.mEndOffset = end + 1;
					}
					break;
				case '\'':
					if (currentToken.mSubType == TokenSubType::CHAR_LITERAL) {
						currentToken.mEndOffset = end + 1;
						endToken(currentToken, tokens);
					} else {
						if (currentToken.mType != TokenType::NOTHING) {
							endToken(currentToken, tokens);
						}
						currentToken.mType = TokenType::LITERAL;
						currentToken.mSubType = TokenSubType::CHAR_LITERAL;
						currentToken.mText.erase();
						currentToken.mStartOffset = start;
						currentToken.mEndOffset = end + 1;
					}
					break;
				case '\\':
					if (currentToken.mSubType == TokenSubType::STRING_LITERAL) {
						currentToken.mType = TokenType::STRING_ESCAPE_SEQUENCE;
						break;
					} else if (currentToken.mSubType == TokenSubType::CHAR_LITERAL) {
						currentToken.mType = TokenType::CHAR_ESCAPE_SEQUENCE;
						break;
					} else { // \ in code is a reference operator
						endToken(currentToken, tokens);
						currentToken.mType = TokenType::OPERATOR;
						currentToken.mSubType = TokenSubType::NOTHING;
						currentToken.mStartOffset = start;
						currentToken.mEndOffset = end + 1;
						currentToken.mText.erase();
						currentToken.mText.append(1, currChar);
						endToken(currentToken, tokens);
					}
					break;
				case ';':
					endToken(currentToken, tokens);
					currentToken.mType = TokenType::SEMICOLON;
					currentToken.mSubType = TokenSubType::NOTHING;
					currentToken.mText.append(1, currChar);
					currentToken.mStartOffset = start;
					currentToken.mEndOffset = end + 1;
					endToken(currentToken, tokens);
					break;
				case ' ':
				case '\t':
					if (currentToken.mType == TokenType::SINGLELINE_COMMENT || currentToken.mSubType == TokenSubType::STRING_LITERAL) {
						currentToken.mText.append(1, currChar);
						break;
					}
					endToken(currentToken, tokens);
					break;
				case '\r':
				case '\n':
					endToken(currentToken, tokens);
					currentToken.mLineNumber++;
					start = 0;
					end = 0;
					break;
				default:
					if (currentToken.mType == TokenType::NOTHING || currentToken.mSubType == TokenSubType::INTEGER_LITERAL || currentToken.mSubType == TokenSubType::FLOAT_LITERAL || currentToken.mType == TokenType::OPERATOR) {
						endToken(currentToken, tokens);
						currentToken.mType = TokenType::IDENTIFIER;
						currentToken.mSubType = TokenSubType::NOTHING;
						currentToken.mText.append(1, currChar);
						currentToken.mStartOffset = start;
						currentToken.mEndOffset = end + 1;
					} else {
						currentToken.mText.append(1, currChar);
						currentToken.mEndOffset = end + 1;
					}
					break;
			}
		}
		endToken(currentToken, tokens);

		return tokens;
	}

	void Tokeniser::endToken(Token& currentToken, std::vector<Token>& tokens) {
		if (currentToken.mType == TokenType::IDENTIFIER) {
			if (currentToken.mText == "return") {
				currentToken.mSubType = TokenSubType::RETURN;
			} else if (currentToken.mText == "break") {
				currentToken.mSubType = TokenSubType::BREAK;
			} else if (currentToken.mText == "skip") {
				currentToken.mSubType = TokenSubType::SKIP;
			} else {
				currentToken.mSubType = TokenSubType::USER_DEFINED;
			}

			tokens.push_back(currentToken);
		} else if (currentToken.mType == TokenType::SINGLELINE_COMMENT) {
			std::cout << "Ignoring comment: " << currentToken.mText << std::endl;
		} else if (currentToken.mType != TokenType::NOTHING) {
			tokens.push_back(currentToken);
		}

		currentToken.mType = TokenType::NOTHING;
		currentToken.mSubType = TokenSubType::NOTHING;
		currentToken.mText.erase();
	}

	Token::Token() {
		mType = TokenType::NOTHING;
		mSubType = TokenSubType::NOTHING;
		mText = "";
		mStartOffset = 0;
		mEndOffset = 0;
		mLineNumber = 0;
		file = "";
	}

	void Token::debugPrint() const {
		std::cout << "Token (" << TokenTypes[int(mType)] << ", " << TokenSubTypes[int(mSubType)]
		<< ", \"" << mText << "\" " << file << ":" << mLineNumber << ":" << mStartOffset << "-" << mEndOffset << ")"
		<< std::endl;
	}

	const char* Token::getType() const {
		return TokenTypes[int(mType)];
	}

	std::ostream& operator<<(std::ostream& os, const Token& t) {
		os << t.file << ":" << t.mLineNumber << ":" << t.mStartOffset ;
		return os;
	}

} // forest
