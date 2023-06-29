#ifndef FOREST_TOKENISER_HPP
#define FOREST_TOKENISER_HPP

#include <vector>
#include <string>

namespace forest::parser {

	enum class TokenType : int {
		NOTHING = 0,
		IDENTIFIER,
		LITERAL,
		OPERATOR,
		STRING_ESCAPE_SEQUENCE,
		CHAR_ESCAPE_SEQUENCE,
		POTENTIAL_COMMENT,
		SINGLELINE_COMMENT,
		MULTILINE_COMMENT,
		SEMICOLON,
		POTENTIAL_NEGATIVE_NUMBER
	};

	enum class TokenSubType : int {
		NOTHING = 0,
		INTEGER_LITERAL,
		FLOAT_LITERAL,
		STRING_LITERAL,
		CHAR_LITERAL,

		// Maybe operator subtypes too
		DOT,
		RANGE,
		NAMESPACE,

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
		"CHAR_ESCAPE_SEQUENCE",
		"POTENTIAL_COMMENT",
		"SINGLELINE_COMMENT",
		"MULTILINE_COMMENT",
		"SEMICOLON",
		"POTENTIAL_NEGATIVE_NUMBER"
	};

	static const char* TokenSubTypes[] = {
		"NOTHING",
		"INTEGER_LITERAL",
		"FLOAT_LITERAL",
		"STRING_LITERAL",
		"CHAR_LITERAL",
		"DOT",
		"RANGE",
		"NAMESPACE",
		"RETURN",
		"BREAK",
		"SKIP",
		"USER_DEFINED"
	};

	class Token {
	public:
		TokenType mType {TokenType::NOTHING};
		TokenSubType mSubType {TokenSubType::NOTHING};
		std::string mText;
		size_t mStartOffset{0};
		size_t mEndOffset{0};
		size_t mLineNumber{1};
		std::string file;

		Token();

		void debugPrint() const;
		const char* getType() const;
		friend std::ostream& operator<<(std::ostream&, const Token&);
	};

	class Tokeniser {
	public:
		static std::vector<Token> parse(const std::string& inProgram, const std::string& fileName);

		static void endToken(Token& token, std::vector<Token>& tokens);
	};

	std::ostream& operator<<(std::ostream&, const Token&);
} // forest

#endif //FOREST_TOKENISER_HPP
