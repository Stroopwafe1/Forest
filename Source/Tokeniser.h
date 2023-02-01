#ifndef FOREST_TOKENISER_H
#define FOREST_TOKENISER_H

#include <vector>
#include <string>

namespace forest::parser {

	enum TokenType {
		NOTHING = 0,
		IDENTIFIER,
		LITERAL,
		OPERATOR,
		STRING_ESCAPE_SEQUENCE,
		POTENTIAL_COMMENT,
		COMMENT
	};

	enum TokenSubType {
		NONE = 0,
		INTEGER_LITERAL,
		FLOAT_LITERAL,
		STRING_LITERAL,

		// Maybe operator subtypes too

		// Keyword Identifiers
		RETURN,
		BREAK,
		SKIP,
		USER_DEFINED
	};

	static const char* TokenTypes[] = {
		"NOTHING",
		"IDENTIFIER",
		"LITERAL",
		"OPERATOR",
		"STRING_ESCAPE_SEQUENCE",
		"POTENTIAL_COMMENT",
		"COMMENT"
	};

	static const char* TokenSubTypes[] = {
		"NONE",
		"INTEGER_LITERAL",
		"FLOAT_LITERAL",
		"STRING_LITERAL",
		"RETURN",
		"BREAK",
		"SKIP",
		"USER_DEFINED"
	};

	class Token {
	public:
		enum TokenType mType {NOTHING};
		enum TokenSubType mSubType {NONE};
		std::string mText;
		size_t mStartOffset{0};
		size_t mEndOffset{0};
		size_t mLineNumber{1};

		void debugPrint() const;
	};

	class Tokeniser {
	public:
		std::vector<Token> parse(const std::string& inProgram);

		void endToken(Token& token, std::vector<Token>& tokens);
	};

} // forest

#endif //FOREST_TOKENISER_H
