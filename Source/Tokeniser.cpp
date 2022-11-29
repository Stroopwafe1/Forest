//
// Created by Lilith Littlewood on 05/08/2022.
//

#include <stdexcept>
#include <iostream>
#include "Tokeniser.h"

namespace simpleparser {
	std::vector<Token> Tokeniser::parse(const std::string &inProgram) {
		std::vector<Token> tokens;
		Token currentToken;
		
		for (int x = 0; x < inProgram.length(); x++) {
			char currChar = inProgram[x];

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
				currentToken.mEndOffset = x;
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
						currentToken.mStartOffset = currentToken.mEndOffset = x;
						currentToken.mText.erase();
						currentToken.mText.append(1, currChar);
					} else {
						currentToken.mEndOffset = x;
						currentToken.mText.append(1, currChar);
					}
					break;

				case '.':
					if (currentToken.mType == NOTHING || currentToken.mType == IDENTIFIER) {
						endToken(currentToken, tokens);
						currentToken.mType = OPERATOR;
						currentToken.mSubType = NONE;
						currentToken.mStartOffset = currentToken.mEndOffset = x;
						currentToken.mText.append(1, currChar);
						endToken(currentToken, tokens);
					} else if (currentToken.mSubType == FLOAT_LITERAL) {// A *second* decimal point
						// TODO: Change this to RANGE operator
						endToken(currentToken, tokens);
						// Already taken care of by the endToken function
					} else if (currentToken.mSubType == INTEGER_LITERAL) {// Integers can't contain dots, so this must be a decimal number.
						currentToken.mSubType = FLOAT_LITERAL;
						currentToken.mEndOffset = x;
						currentToken.mText.append(1, currChar);
					} else if (currentToken.mType != NOTHING) {
						currentToken.mEndOffset = x;
						currentToken.mText.append(1, currChar);
					}
					break;

				case '/':
					if (currentToken.mSubType == STRING_LITERAL) {
						currentToken.mEndOffset = x;
						currentToken.mText.append(1, currChar);
					} else if (currentToken.mType == POTENTIAL_COMMENT) {
						currentToken.mType = COMMENT;
						currentToken.mSubType = NONE;
						currentToken.mText.erase();
					} else {
						endToken(currentToken, tokens);
						currentToken.mType = POTENTIAL_COMMENT;
						currentToken.mSubType = NONE;
						currentToken.mStartOffset = currentToken.mEndOffset = x;
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
						currentToken.mStartOffset = currentToken.mEndOffset = x;
						currentToken.mText.erase();
						currentToken.mText.append(1, currChar);
						endToken(currentToken, tokens);
					} else {
						currentToken.mText.append(1, currChar);
						currentToken.mEndOffset = x;
					}
					break;
				case '"':
					if (currentToken.mSubType == STRING_LITERAL) {
						currentToken.mEndOffset = x;
						endToken(currentToken, tokens);
					} else {
						if (currentToken.mType != NOTHING) {
							endToken(currentToken, tokens);
						}
						currentToken.mType = LITERAL;
						currentToken.mSubType = STRING_LITERAL;
						currentToken.mText.erase();
						currentToken.mStartOffset = currentToken.mEndOffset = x;
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
						currentToken.mStartOffset = currentToken.mEndOffset = x;
						currentToken.mText.erase();
						currentToken.mText.append(1, currChar);
						endToken(currentToken, tokens);
					}
					break;
				case ';':
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
					break;
				default:
					if (currentToken.mType == NOTHING || currentToken.mSubType == INTEGER_LITERAL || currentToken.mSubType == FLOAT_LITERAL) {
						endToken(currentToken, tokens);
						currentToken.mType = IDENTIFIER;
						currentToken.mSubType = NONE;
						currentToken.mText.append(1, currChar);
						currentToken.mStartOffset = currentToken.mEndOffset = x;
					} else {
						currentToken.mText.append(1, currChar);
						currentToken.mEndOffset = x;
					}
					break;
			}
		}
		endToken(currentToken, tokens);

		return tokens;
	}

	void Tokeniser::endToken(Token& currentToken, std::vector<Token>& tokens) {
		if (currentToken.mType == LITERAL && currentToken.mSubType != STRING_LITERAL) {
			if (currentToken.mSubType == FLOAT_LITERAL && (*currentToken.mText.end()) == '.') {
				currentToken.mSubType = INTEGER_LITERAL;
				currentToken.mText = currentToken.mText.substr(0, currentToken.mText.size() - 1);
				currentToken.mEndOffset -= 1;
				tokens.push_back(currentToken);

				currentToken.mType = OPERATOR;
				currentToken.mSubType = NONE;
				currentToken.mText = ".";
				currentToken.mStartOffset = currentToken.mEndOffset;
				tokens.push_back(currentToken);
			} else {
				tokens.push_back(currentToken);
			}
		} else if (currentToken.mType == IDENTIFIER) {
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
		<< ", \"" << mText << "\" " << mLineNumber << ":" << mStartOffset << "-" << mEndOffset << ")"
		<< std::endl;
	}
} // simpleparser
