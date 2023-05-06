#include <gtest/gtest.h>
#include "Tokeniser.hpp"

using namespace forest::parser;

class TokeniserTests : public ::testing::Test {
public:
	std::string filePath = "testing_file.tree";

	void SetUp() override {

	}

	void TearDown() override {

	}
};

TEST_F(TokeniserTests, TokeniserParseShouldTokeniseStringLiterals) {
	std::string code = "\"This is a string. Strings can contain numbers like 0128, characters like .;/@!~-{}[]<>', and other things\"";
	std::vector<Token> tokens = forest::parser::Tokeniser::parse(code, filePath);

	ASSERT_EQ(tokens.size(), 1);

	Token first = tokens[0];
	EXPECT_EQ(first.mType, TokenType::LITERAL);
	EXPECT_EQ(first.mSubType, TokenSubType::STRING_LITERAL);
	EXPECT_STREQ(first.mText.c_str(), "This is a string. Strings can contain numbers like 0128, characters like .;/@!~-{}[]<>', and other things");
}

TEST_F(TokeniserTests, TokeniserParseShouldTokeniseStringLiteralsWithEscapeSequences) {
	std::string code = R"("This is a string with an escape \t sequence")";
	std::vector<Token> tokens = forest::parser::Tokeniser::parse(code, filePath);

	ASSERT_EQ(tokens.size(), 1);

	Token first = tokens[0];
	EXPECT_EQ(first.mType, TokenType::LITERAL);
	EXPECT_EQ(first.mSubType, TokenSubType::STRING_LITERAL);
	EXPECT_STREQ(first.mText.c_str(), "This is a string with an escape \t sequence");
}

TEST_F(TokeniserTests, TokeniserParseShouldTokeniseIntegerLiterals) {
	std::string code = "1203 -928";
	std::vector<Token> tokens = forest::parser::Tokeniser::parse(code, filePath);

	ASSERT_EQ(tokens.size(), 2);

	Token first = tokens[0];
	EXPECT_EQ(first.mType, TokenType::LITERAL);
	EXPECT_EQ(first.mSubType, TokenSubType::INTEGER_LITERAL);
	EXPECT_STREQ(first.mText.c_str(), "1203");

	Token second = tokens[1];
	EXPECT_EQ(second.mType, TokenType::LITERAL);
	EXPECT_EQ(second.mSubType, TokenSubType::INTEGER_LITERAL);
	EXPECT_STREQ(second.mText.c_str(), "-928");
}

TEST_F(TokeniserTests, TokeniserParseShouldTokeniseFloatLiterals) {
	std::string code = "1203.102 -928.291";
	std::vector<Token> tokens = forest::parser::Tokeniser::parse(code, filePath);

	ASSERT_EQ(tokens.size(), 2);

	Token first = tokens[0];
	EXPECT_EQ(first.mType, TokenType::LITERAL);
	EXPECT_EQ(first.mSubType, TokenSubType::FLOAT_LITERAL);
	EXPECT_STREQ(first.mText.c_str(), "1203.102");

	Token second = tokens[1];
	EXPECT_EQ(second.mType, TokenType::LITERAL);
	EXPECT_EQ(second.mSubType, TokenSubType::FLOAT_LITERAL);
	EXPECT_STREQ(second.mText.c_str(), "-928.291");
}

TEST_F(TokeniserTests, TokeniserParseShouldTokeniseRange) {
	std::string code = "1..4";
	std::vector<Token> tokens = forest::parser::Tokeniser::parse(code, filePath);

	ASSERT_EQ(tokens.size(), 3);

	Token first = tokens[0];
	EXPECT_EQ(first.mType, TokenType::LITERAL);
	EXPECT_EQ(first.mSubType, TokenSubType::INTEGER_LITERAL);
	EXPECT_STREQ(first.mText.c_str(), "1");

	Token second = tokens[1];
	EXPECT_EQ(second.mType, TokenType::OPERATOR);
	EXPECT_EQ(second.mSubType, TokenSubType::RANGE);
	EXPECT_STREQ(second.mText.c_str(), "..");

	Token third = tokens[2];
	EXPECT_EQ(third.mType, TokenType::LITERAL);
	EXPECT_EQ(third.mSubType, TokenSubType::INTEGER_LITERAL);
	EXPECT_STREQ(third.mText.c_str(), "4");
}

TEST_F(TokeniserTests, TokeniserEndTokenShouldConvertSubtypes) {
	std::vector<Token> tokens;

	Token token;
	token.mType = TokenType::IDENTIFIER;
	token.mText = "return";

	forest::parser::Tokeniser::endToken(token, tokens);
	ASSERT_EQ(tokens.size(), 1);

	Token returnToken = tokens[0];
	EXPECT_EQ(returnToken.mType, TokenType::IDENTIFIER);
	EXPECT_EQ(returnToken.mSubType, TokenSubType::RETURN);
	EXPECT_STREQ(returnToken.mText.c_str(), "return");

	EXPECT_EQ(token.mType, TokenType::NOTHING);
	EXPECT_EQ(token.mSubType, TokenSubType::NOTHING);
	EXPECT_STREQ(token.mText.c_str(), "");

	token.mType = TokenType::IDENTIFIER;
	token.mText = "skip";

	forest::parser::Tokeniser::endToken(token, tokens);
	ASSERT_EQ(tokens.size(), 2);

	Token skipToken = tokens[1];
	EXPECT_EQ(skipToken.mType, TokenType::IDENTIFIER);
	EXPECT_EQ(skipToken.mSubType, TokenSubType::SKIP);
	EXPECT_STREQ(skipToken.mText.c_str(), "skip");
}

TEST(TokenTests, TokenGetTypeShouldReturnType) {
	Token token;
	token.mType = TokenType::LITERAL;
	EXPECT_STREQ(token.getType(), "LITERAL");

	token.mType = TokenType::IDENTIFIER;
	EXPECT_STREQ(token.getType(), "IDENTIFIER");

	token.mType = TokenType::NOTHING;
	EXPECT_STREQ(token.getType(), "NOTHING");

	token.mType = TokenType::OPERATOR;
	EXPECT_STREQ(token.getType(), "OPERATOR");

	token.mType = TokenType::SEMICOLON;
	EXPECT_STREQ(token.getType(), "SEMICOLON");

	token.mType = TokenType::COMMENT;
	EXPECT_STREQ(token.getType(), "COMMENT");

	token.mType = TokenType::POTENTIAL_COMMENT;
	EXPECT_STREQ(token.getType(), "POTENTIAL_COMMENT");

	token.mType = TokenType::POTENTIAL_NEGATIVE_NUMBER;
	EXPECT_STREQ(token.getType(), "POTENTIAL_NEGATIVE_NUMBER");

	token.mType = TokenType::STRING_ESCAPE_SEQUENCE;
	EXPECT_STREQ(token.getType(), "STRING_ESCAPE_SEQUENCE");
}