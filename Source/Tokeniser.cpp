#include <stdexcept>
#include <iostream>
#include <sstream>
#include "Tokeniser.h"

namespace forest::parser {
	std::vector<Token> Tokeniser::parse(const std::string &inProgram, const std::string& fileName) {
		std::vector<Token> tokens;
		Token currentToken;
		currentToken.file = fileName;
		
		int start = 0;
		int end = 0;
		for (char currChar : inProgram) {
			start++;
			end++;
			Token lastToken {};
			if (!tokens.empty())
				lastToken = tokens.back();

			if (currentToken.mType == STRING_ESCAPE_SEQUENCE) {
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
						" in string at " + std::to_string(currentToken.mLineNumber) + ":" + std::to_string(currentToken.mEndOffset));
				}
				currentToken.mType = LITERAL;
				currentToken.mSubType = STRING_LITERAL;
				currentToken.mEndOffset = end + 1;
				continue;
			} else if (currentToken.mType == POTENTIAL_COMMENT && currChar != '/') {
				currentToken.mType = OPERATOR;
				currentToken.mSubType = NONE;
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
					if (currentToken.mType == NOTHING) {
						currentToken.mType = LITERAL;
						currentToken.mSubType = INTEGER_LITERAL;
						currentToken.mStartOffset = start;
						currentToken.mEndOffset = end + 1;
						currentToken.mText.erase();
						currentToken.mText.append(1, currChar);
					} else {
						currentToken.mEndOffset = end + 1;
						currentToken.mText.append(1, currChar);
					}
					break;

				case '.':
					if ((currentToken.mType == NOTHING || currentToken.mType == IDENTIFIER) && lastToken.mSubType != DOT) {
						endToken(currentToken, tokens);
						currentToken.mType = OPERATOR;
						currentToken.mSubType = DOT;
						currentToken.mStartOffset = start;
						currentToken.mEndOffset = end + 1;
						currentToken.mText.append(1, currChar);
						endToken(currentToken, tokens);
					} else if (currentToken.mSubType == FLOAT_LITERAL) {// A *second* decimal point -> range operator
						// Split into 2 tokens, integer literal and range op
						currentToken.mText.erase(currentToken.mText.size() - 1);
						currentToken.mSubType = INTEGER_LITERAL;
						currentToken.mEndOffset = start - 2;
						endToken(currentToken, tokens);

						currentToken.mType = OPERATOR;
						currentToken.mSubType = RANGE;
						currentToken.mStartOffset = start;
						currentToken.mEndOffset = end + 1;
						currentToken.mText.append(2, currChar);
						endToken(currentToken, tokens);
					} else if (lastToken.mSubType == DOT) { // A 2nd dot -> range operator
						tokens.pop_back();
						currentToken.mText.append(2, currChar);
						currentToken.mType = OPERATOR;
						currentToken.mSubType = RANGE;
						currentToken.mStartOffset = start;
						currentToken.mEndOffset = end + 1;
						endToken(currentToken, tokens);
					} else if (currentToken.mSubType == INTEGER_LITERAL) {// Integers can't contain dots, so this must be a decimal number.
						currentToken.mSubType = FLOAT_LITERAL;
						currentToken.mEndOffset = end + 1;
						currentToken.mText.append(1, currChar);
					} else if (currentToken.mType != NOTHING) {
						currentToken.mEndOffset = end + 1;
						currentToken.mText.append(1, currChar);
					}
					break;

				case '/':
					if (currentToken.mSubType == STRING_LITERAL) {
						currentToken.mEndOffset = end + 1;
						currentToken.mText.append(1, currChar);
					} else if (currentToken.mType == POTENTIAL_COMMENT) {
						currentToken.mType = COMMENT;
						currentToken.mSubType = NONE;
						currentToken.mText.erase();
					} else {
						endToken(currentToken, tokens);
						currentToken.mType = POTENTIAL_COMMENT;
						currentToken.mSubType = NONE;
						currentToken.mStartOffset = start;
						currentToken.mEndOffset = end + 1;
						currentToken.mText.append(1, currChar);
					}

					break;

				case '{':
				case '}':
				case '(':
				case ')':
				case '[':
				case ']':
				case '<':
				case '>':
				case '=':
				case '+':
				case '-':
				case '*':
				case ',':
					if (currentToken.mSubType != STRING_LITERAL) {
						endToken(currentToken, tokens);
						currentToken.mType = OPERATOR;
						currentToken.mSubType = NONE;
						currentToken.mStartOffset = start;
						currentToken.mEndOffset = end + 1;
						currentToken.mText.erase();
						currentToken.mText.append(1, currChar);
						endToken(currentToken, tokens);
					} else {
						currentToken.mText.append(1, currChar);
						currentToken.mEndOffset = end + 1;
					}
					break;
				case '"':
					if (currentToken.mSubType == STRING_LITERAL) {
						currentToken.mEndOffset = end + 1;
						endToken(currentToken, tokens);
					} else {
						if (currentToken.mType != NOTHING) {
							endToken(currentToken, tokens);
						}
						currentToken.mType = LITERAL;
						currentToken.mSubType = STRING_LITERAL;
						currentToken.mText.erase();
						currentToken.mStartOffset = start;
						currentToken.mEndOffset = end + 1;
					}
					break;
				case '\\':
					if (currentToken.mSubType == STRING_LITERAL) {
						currentToken.mType = STRING_ESCAPE_SEQUENCE;
						currentToken.mSubType = NONE;
					} else { // \ in code is a reference operator
						endToken(currentToken, tokens);
						currentToken.mType = OPERATOR;
						currentToken.mSubType = NONE;
						currentToken.mStartOffset = start;
						currentToken.mEndOffset = end + 1;
						currentToken.mText.erase();
						currentToken.mText.append(1, currChar);
						endToken(currentToken, tokens);
					}
					break;
				case ';':
					endToken(currentToken, tokens);
					currentToken.mType = SEMICOLON;
					currentToken.mSubType = NONE;
					currentToken.mText.append(1, currChar);
					currentToken.mStartOffset = start;
					currentToken.mEndOffset = end + 1;
					endToken(currentToken, tokens);
					break;
				case ' ':
				case '\t':
					if (currentToken.mType == COMMENT || currentToken.mSubType == STRING_LITERAL) {
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
					if (currentToken.mType == NOTHING || currentToken.mSubType == INTEGER_LITERAL || currentToken.mSubType == FLOAT_LITERAL) {
						endToken(currentToken, tokens);
						currentToken.mType = IDENTIFIER;
						currentToken.mSubType = NONE;
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
		if (currentToken.mType == IDENTIFIER) {
			if (currentToken.mText == "return") {
				currentToken.mSubType = RETURN;
			} else if (currentToken.mText == "break") {
				currentToken.mSubType = BREAK;
			} else if (currentToken.mText == "skip") {
				currentToken.mSubType = SKIP;
			} else {
				currentToken.mSubType = USER_DEFINED;
			}

			tokens.push_back(currentToken);
		} else if (currentToken.mType == COMMENT) {
			std::cout << "Ignoring comment: " << currentToken.mText << std::endl;
		} else if (currentToken.mType != NOTHING) {
			tokens.push_back(currentToken);
		}

		currentToken.mType = NOTHING;
		currentToken.mSubType = NONE;
		currentToken.mText.erase();
	}

	void Token::debugPrint() const {
		std::cout << "Token (" << TokenTypes[mType] << ", " << TokenSubTypes[mSubType]
		<< ", \"" << mText << "\" " << file << ":" << mLineNumber << ":" << mStartOffset << "-" << mEndOffset << ")"
		<< std::endl;
	}

	std::ostream& operator<<(std::ostream& os, const Token& t) {
		os << t.file << ":" << t.mLineNumber << ":" << t.mStartOffset ;
		return os;
	}

	const char* Token::getType() const {
		return TokenTypes[mType];
	}
} // forest
