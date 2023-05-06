#include <gtest/gtest.h>
#include "Parser.hpp"

using namespace forest::parser;

class ParserTests : public ::testing::Test {

	void SetUp() override {

	}

	void TearDown() override {

	}

protected:
	Parser parser;
};

TEST_F(ParserTests, ParserParseBareMainShouldParse) {
	std::string filePath = "testing.tree";
	std::string code = "i32 main(string[] argv) { return 0; }";
	std::vector<Token> tokens = Tokeniser::parse(code, filePath);

	Programme programme = parser.parse(tokens);

	ASSERT_EQ(programme.functions.size(), 1);
	ASSERT_TRUE(programme.literals.empty());
	ASSERT_FALSE(programme.requires_libs);

	Function function = programme.functions[0];

	EXPECT_EQ(function.mReturnType.builtinType, Builtin_Type::I32);
	EXPECT_STREQ(function.mReturnType.name.c_str(), "i32");

	EXPECT_STREQ(function.mName.c_str(), "main");

	ASSERT_EQ(function.mArgs.size(), 1);
	FuncArg arg = function.mArgs[0];
	EXPECT_EQ(arg.mType.builtinType, Builtin_Type::ARRAY);
	EXPECT_STREQ(arg.mName.c_str(), "argv");

	Block block = function.mBody;
	ASSERT_EQ(block.statements.size(), 1);
	Statement statement = block.statements[0];
	EXPECT_EQ(statement.mType, Statement_Type::RETURN_CALL);
	EXPECT_STREQ(statement.mContent->mValue.mText.c_str(), "0");
	ASSERT_FALSE(statement.funcCall.has_value());
	ASSERT_FALSE(statement.loopStatement.has_value());
}

TEST_F(ParserTests, ParserPeekNextTokenShouldReturnNullOnEnd) {
	std::vector<Token> tokens = Tokeniser::parse("1", "testing.tree");;
	parser.mCurrentToken = tokens.begin();
	parser.mTokensEnd = tokens.end();

	std::optional<Token> nextToken = parser.peekNextToken();
	ASSERT_FALSE(nextToken.has_value());
}

TEST_F(ParserTests, ParserPeekNextTokenShouldReturnValue) {
	std::vector<Token> tokens = Tokeniser::parse("1 2", "testing.tree");
	parser.mCurrentToken = tokens.begin();
	parser.mTokensEnd = tokens.end();

	std::optional<Token> nextToken = parser.peekNextToken();
	ASSERT_TRUE(nextToken.has_value());

	EXPECT_EQ(nextToken.value().mType, TokenType::LITERAL);
	EXPECT_EQ(nextToken.value().mSubType, TokenSubType::INTEGER_LITERAL);
	EXPECT_STREQ(nextToken.value().mText.c_str(), "2");
}

TEST_F(ParserTests, ParserExpectIdentifierShouldReturnAnyIdentifier) {
	std::vector<Token> tokens = Tokeniser::parse("main", "testing.tree");
	parser.mCurrentToken = tokens.begin();
	parser.mTokensEnd = tokens.end();

	std::optional<Token> nextToken = parser.expectIdentifier();
	ASSERT_TRUE(nextToken.has_value());

	EXPECT_EQ(nextToken.value().mType, TokenType::IDENTIFIER);
	EXPECT_EQ(nextToken.value().mSubType, TokenSubType::USER_DEFINED);
	EXPECT_STREQ(nextToken.value().mText.c_str(), "main");
}

TEST_F(ParserTests, ParserExpectIdentifierShouldReturnSpecifiedIdentifier) {
	std::vector<Token> tokens = Tokeniser::parse("main", "testing.tree");
	parser.mCurrentToken = tokens.begin();
	parser.mTokensEnd = tokens.end();

	std::optional<Token> nextToken = parser.expectIdentifier("main");
	ASSERT_TRUE(nextToken.has_value());

	EXPECT_EQ(nextToken.value().mType, TokenType::IDENTIFIER);
	EXPECT_EQ(nextToken.value().mSubType, TokenSubType::USER_DEFINED);
	EXPECT_STREQ(nextToken.value().mText.c_str(), "main");
}

TEST_F(ParserTests, ParserExpectIdentifierShouldNotReturnSpecifiedIdentifier) {
	std::vector<Token> tokens = Tokeniser::parse("main", "testing.tree");
	parser.mCurrentToken = tokens.begin();
	parser.mTokensEnd = tokens.end();

	std::optional<Token> nextToken = parser.expectIdentifier("blep");
	ASSERT_FALSE(nextToken.has_value());
}

TEST_F(ParserTests, ParserExpectOperatorShouldReturnAnyOperator) {
	std::vector<Token> tokens = Tokeniser::parse("{", "testing.tree");
	parser.mCurrentToken = tokens.begin();
	parser.mTokensEnd = tokens.end();

	std::optional<Token> nextToken = parser.expectOperator();
	ASSERT_TRUE(nextToken.has_value());

	EXPECT_EQ(nextToken.value().mType, TokenType::OPERATOR);
	EXPECT_STREQ(nextToken.value().mText.c_str(), "{");
}

TEST_F(ParserTests, ParserExpectOperatorShouldReturnSpecifiedOperator) {
	std::vector<Token> tokens = Tokeniser::parse("{", "testing.tree");
	parser.mCurrentToken = tokens.begin();
	parser.mTokensEnd = tokens.end();

	std::optional<Token> nextToken = parser.expectOperator("{");
	ASSERT_TRUE(nextToken.has_value());

	EXPECT_EQ(nextToken.value().mType, TokenType::OPERATOR);
	EXPECT_STREQ(nextToken.value().mText.c_str(), "{");
}

TEST_F(ParserTests, ParserExpectOperatorShouldNotReturnSpecifiedOperator) {
	std::vector<Token> tokens = Tokeniser::parse("{", "testing.tree");
	parser.mCurrentToken = tokens.begin();
	parser.mTokensEnd = tokens.end();

	std::optional<Token> nextToken = parser.expectOperator("]");
	ASSERT_FALSE(nextToken.has_value());
}

TEST_F(ParserTests, ParserExpectLiteralShouldReturnAnyLiteral) {
	std::vector<Token> tokens = Tokeniser::parse("1", "testing.tree");
	parser.mCurrentToken = tokens.begin();
	parser.mTokensEnd = tokens.end();

	std::optional<Token> nextToken = parser.expectLiteral();
	ASSERT_TRUE(nextToken.has_value());

	EXPECT_EQ(nextToken.value().mType, TokenType::LITERAL);
	EXPECT_EQ(nextToken.value().mSubType, TokenSubType::INTEGER_LITERAL);
	EXPECT_STREQ(nextToken.value().mText.c_str(), "1");
}

TEST_F(ParserTests, ParserExpectLiteralShouldReturnSpecifiedLiteral) {
	std::vector<Token> tokens = Tokeniser::parse("1", "testing.tree");
	parser.mCurrentToken = tokens.begin();
	parser.mTokensEnd = tokens.end();

	std::optional<Token> nextToken = parser.expectLiteral("1");
	ASSERT_TRUE(nextToken.has_value());

	EXPECT_EQ(nextToken.value().mType, TokenType::LITERAL);
	EXPECT_EQ(nextToken.value().mSubType, TokenSubType::INTEGER_LITERAL);
	EXPECT_STREQ(nextToken.value().mText.c_str(), "1");
}

TEST_F(ParserTests, ParserExpectLiteralShouldNotReturnSpecifiedLiteral) {
	std::vector<Token> tokens = Tokeniser::parse("1", "testing.tree");
	parser.mCurrentToken = tokens.begin();
	parser.mTokensEnd = tokens.end();

	std::optional<Token> nextToken = parser.expectLiteral("2");
	ASSERT_FALSE(nextToken.has_value());
}

TEST_F(ParserTests, ParserExpectSemicolonShouldReturnSpecifiedSemicolon) {
	std::vector<Token> tokens = Tokeniser::parse(";", "testing.tree");
	parser.mCurrentToken = tokens.begin();
	parser.mTokensEnd = tokens.end();

	std::optional<Token> nextToken = parser.expectSemicolon();
	ASSERT_TRUE(nextToken.has_value());

	EXPECT_EQ(nextToken.value().mType, TokenType::SEMICOLON);
	EXPECT_STREQ(nextToken.value().mText.c_str(), ";");
}

TEST_F(ParserTests, ParserExpectSemicolonShouldNotReturnOnNoSemicolon) {
	std::vector<Token> tokens = Tokeniser::parse("!", "testing.tree");
	parser.mCurrentToken = tokens.begin();
	parser.mTokensEnd = tokens.end();

	std::optional<Token> nextToken = parser.expectSemicolon();
	ASSERT_FALSE(nextToken.has_value());
}

TEST_F(ParserTests, ParserTryParseExpression4Minus3Plus1) {
	std::vector<Token> tokens = Tokeniser::parse("(4 - 3) + 1;", "testing.tree");
	parser.mCurrentToken = tokens.begin();
	parser.mTokensEnd = tokens.end();

	Statement s;
	s.mType = Statement_Type::VAR_DECLARATION;
	Expression* expression = parser.expectExpression(s);
	ASSERT_NE(expression, nullptr);

	EXPECT_EQ(expression->mValue.mType, TokenType::OPERATOR);
	EXPECT_STREQ(expression->mValue.mText.c_str(), "+");

	Token left = expression->mLeft->mValue;
	EXPECT_EQ(left.mType, TokenType::OPERATOR);
	EXPECT_STREQ(left.mText.c_str(), "-");

	Token leftleft = expression->mLeft->mLeft->mValue;
	EXPECT_EQ(leftleft.mType, TokenType::LITERAL);
	EXPECT_EQ(leftleft.mSubType, TokenSubType::INTEGER_LITERAL);
	EXPECT_STREQ(leftleft.mText.c_str(), "4");

	Token leftright = expression->mLeft->mRight->mValue;
	EXPECT_EQ(leftright.mType, TokenType::LITERAL);
	EXPECT_EQ(leftright.mSubType, TokenSubType::INTEGER_LITERAL);
	EXPECT_STREQ(leftright.mText.c_str(), "3");

	Token right = expression->mRight->mValue;
	EXPECT_EQ(right.mType, TokenType::LITERAL);
	EXPECT_EQ(right.mSubType, TokenSubType::INTEGER_LITERAL);
	EXPECT_STREQ(right.mText.c_str(), "1");

	// Assert that it hasn't consumed the semicolon
	ASSERT_EQ((*parser.mCurrentToken).mType, TokenType::SEMICOLON);
}

TEST_F(ParserTests, ParserTryParseExpression4Minus3Plus1V2) {
	std::vector<Token> tokens = Tokeniser::parse("4 - (3 + 1);", "testing.tree");
	parser.mCurrentToken = tokens.begin();
	parser.mTokensEnd = tokens.end();

	Statement s;
	s.mType = Statement_Type::VAR_DECLARATION;
	Expression* expression = parser.expectExpression(s);
	ASSERT_NE(expression, nullptr);

	EXPECT_EQ(expression->mValue.mType, TokenType::OPERATOR);
	EXPECT_STREQ(expression->mValue.mText.c_str(), "-");

	Token left = expression->mLeft->mValue;
	EXPECT_EQ(left.mType, TokenType::LITERAL);
	EXPECT_EQ(left.mSubType, TokenSubType::INTEGER_LITERAL);
	EXPECT_STREQ(left.mText.c_str(), "4");

	Token right = expression->mRight->mValue;
	EXPECT_EQ(right.mType, TokenType::OPERATOR);
	EXPECT_STREQ(right.mText.c_str(), "+");

	Token rightleft = expression->mRight->mLeft->mValue;
	EXPECT_EQ(rightleft.mType, TokenType::LITERAL);
	EXPECT_EQ(rightleft.mSubType, TokenSubType::INTEGER_LITERAL);
	EXPECT_STREQ(rightleft.mText.c_str(), "3");

	Token rightright = expression->mRight->mRight->mValue;
	EXPECT_EQ(rightright.mType, TokenType::LITERAL);
	EXPECT_EQ(rightright.mSubType, TokenSubType::INTEGER_LITERAL);
	EXPECT_STREQ(rightright.mText.c_str(), "1");

	// Assert that it hasn't consumed the semicolon
	ASSERT_EQ((*parser.mCurrentToken).mType, TokenType::SEMICOLON);
}

TEST_F(ParserTests, ParserExpressionCollapse4Minus1) {
	std::vector<Token> tokens = Tokeniser::parse("4 - 1;", "testing.tree");
	parser.mCurrentToken = tokens.begin();
	parser.mTokensEnd = tokens.end();

	Statement s;
	s.mType = Statement_Type::VAR_DECLARATION;
	Expression* expression = parser.expectExpression(s);
	ASSERT_NE(expression, nullptr);
	expression->Collapse();

	ASSERT_NE(expression, nullptr);
	ASSERT_EQ(expression->mLeft, nullptr);
	ASSERT_EQ(expression->mRight, nullptr);

	EXPECT_EQ(expression->mValue.mType, TokenType::LITERAL);
	EXPECT_EQ(expression->mValue.mSubType, TokenSubType::INTEGER_LITERAL);
	EXPECT_STREQ(expression->mValue.mText.c_str(), "3");
}

TEST_F(ParserTests, ParserExpressionCollapse4Minus3Plus1) {
	std::vector<Token> tokens = Tokeniser::parse("(4 * 3) + 1;", "testing.tree");
	parser.mCurrentToken = tokens.begin();
	parser.mTokensEnd = tokens.end();

	Statement s;
	s.mType = Statement_Type::VAR_DECLARATION;
	Expression* expression = parser.expectExpression(s);
	ASSERT_NE(expression, nullptr);
	expression->Collapse();

	ASSERT_NE(expression, nullptr);
	ASSERT_EQ(expression->mLeft, nullptr);
	ASSERT_EQ(expression->mRight, nullptr);

	EXPECT_EQ(expression->mValue.mType, TokenType::LITERAL);
	EXPECT_EQ(expression->mValue.mSubType, TokenSubType::INTEGER_LITERAL);
	EXPECT_STREQ(expression->mValue.mText.c_str(), "13");
}

TEST_F(ParserTests, ParserExpressionCollapse4Modulo3) {
	std::vector<Token> tokens = Tokeniser::parse("4 % 3;", "testing.tree");
	parser.mCurrentToken = tokens.begin();
	parser.mTokensEnd = tokens.end();

	Statement s;
	s.mType = Statement_Type::VAR_DECLARATION;
	Expression* expression = parser.expectExpression(s);
	ASSERT_NE(expression, nullptr);
	expression->Collapse();

	ASSERT_NE(expression, nullptr);
	ASSERT_EQ(expression->mLeft, nullptr);
	ASSERT_EQ(expression->mRight, nullptr);

	EXPECT_EQ(expression->mValue.mType, TokenType::LITERAL);
	EXPECT_EQ(expression->mValue.mSubType, TokenSubType::INTEGER_LITERAL);
	EXPECT_STREQ(expression->mValue.mText.c_str(), "1");
}

TEST_F(ParserTests, ParserExpressionCollapse3TimesString) {
	std::vector<Token> tokens = Tokeniser::parse("3 * \"Hello\";", "testing.tree");
	parser.mCurrentToken = tokens.begin();
	parser.mTokensEnd = tokens.end();

	Statement s;
	s.mType = Statement_Type::VAR_DECLARATION;
	Expression* expression = parser.expectExpression(s);
	ASSERT_NE(expression, nullptr);
	expression->Collapse();

	ASSERT_NE(expression, nullptr);
	ASSERT_EQ(expression->mLeft, nullptr);
	ASSERT_EQ(expression->mRight, nullptr);

	EXPECT_EQ(expression->mValue.mType, TokenType::LITERAL);
	EXPECT_EQ(expression->mValue.mSubType, TokenSubType::STRING_LITERAL);
	EXPECT_STREQ(expression->mValue.mText.c_str(), "HelloHelloHello");
}

TEST_F(ParserTests, ParserTryParseReturnStatement) {
	std::vector<Token> tokens = Tokeniser::parse("return 0;", "testing.tree");
	parser.mCurrentToken = tokens.begin();
	parser.mTokensEnd = tokens.end();

	std::optional<Statement> statement = parser.expectStatement();
	ASSERT_TRUE(statement.has_value());

	EXPECT_EQ(statement.value().mType, Statement_Type::RETURN_CALL);
	EXPECT_STREQ(statement.value().mContent->mValue.mText.c_str(), "0");
	EXPECT_FALSE(statement.value().loopStatement.has_value());
	EXPECT_FALSE(statement.value().funcCall.has_value());
}

TEST_F(ParserTests, ParserTryParseLoopStatement) {
	std::vector<Token> tokens = Tokeniser::parse("loop i, 0..10 { stdout.write(i); }", "testing.tree");
	parser.mCurrentToken = tokens.begin();
	parser.mTokensEnd = tokens.end();

	std::optional<Statement> statement = parser.expectStatement();
	ASSERT_TRUE(statement.has_value());

	EXPECT_EQ(statement.value().mType, Statement_Type::LOOP);
	EXPECT_EQ(statement.value().mContent, nullptr);
	ASSERT_TRUE(statement.value().loopStatement.has_value());
	EXPECT_FALSE(statement.value().funcCall.has_value());

	LoopStatement loop = statement.value().loopStatement.value();
	ASSERT_TRUE(loop.mIterator.has_value());
	ASSERT_TRUE(loop.mRange.has_value());

	EXPECT_EQ(loop.mIterator.value().mType.builtinType, Builtin_Type::UI8);
	EXPECT_STREQ(loop.mIterator.value().mName.c_str(), "i");

	EXPECT_EQ(loop.mRange.value().mMinimum, 0);
	EXPECT_EQ(loop.mRange.value().mMaximum, 10);
}

TEST_F(ParserTests, ParserTryParseFuncCallStatement) {
	std::vector<Token> tokens = Tokeniser::parse("stdout.write(20);", "testing.tree");
	parser.mCurrentToken = tokens.begin();
	parser.mTokensEnd = tokens.end();

	std::optional<Statement> statement = parser.expectStatement();
	ASSERT_TRUE(statement.has_value());

	EXPECT_EQ(statement.value().mType, Statement_Type::FUNC_CALL);
	EXPECT_FALSE(statement.value().loopStatement.has_value());
	ASSERT_TRUE(statement.value().funcCall.has_value());

	FuncCallStatement fc = statement.value().funcCall.value();
	EXPECT_EQ(fc.mClassType, StdLib_Class_Type::STDOUT);
	EXPECT_EQ(fc.mFunctionType, StdLib_Function_Type::WRITE);
	ASSERT_EQ(fc.mArgs.size(), 1);

	Expression* arg = fc.mArgs[0];
	EXPECT_EQ(arg->mValue.mType, TokenType::LITERAL);
	EXPECT_EQ(arg->mValue.mSubType, TokenSubType::INTEGER_LITERAL);
	EXPECT_STREQ(arg->mValue.mText.c_str(), "20");
}

TEST_F(ParserTests, ParserTryParseVariableStatement) {
	std::vector<Token> tokens = Tokeniser::parse("ui8 test = 100", "testing.tree");
	parser.mCurrentToken = tokens.begin();
	parser.mTokensEnd = tokens.end();

	std::optional<Statement> statement = parser.expectStatement();
	ASSERT_TRUE(statement.has_value());

	EXPECT_EQ(statement.value().mType, Statement_Type::VAR_ASSIGNMENT);
	EXPECT_STREQ(statement.value().mContent->mValue.mText.c_str(), "100");
	EXPECT_FALSE(statement.value().loopStatement.has_value());
	EXPECT_FALSE(statement.value().funcCall.has_value());
}