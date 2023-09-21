#include "Expression.hpp"

namespace forest::parser {

	Expression::~Expression() {
		for (const auto& exp : mChildren) {
			delete exp;
		}
		mChildren.clear();
	}


	Expression::Expression() {
		mValue = {};
		mChildren = {};
	}

	Expression::Expression(const Expression& rhs) {
		mValue = rhs.mValue;
		for (const auto& exp : rhs.mChildren) {
			Expression* newExp = new Expression(*exp);
			mChildren.push_back(newExp);
		}
	}

	void Expression::Collapse() {
		if (mChildren.empty()) {
			return;
		}

		if (mValue.mType != TokenType::OPERATOR ) {
			return;
		}

		Expression* leftExp = mChildren[0];
		Expression* rightExp = mChildren[1];

		leftExp->Collapse();
		rightExp->Collapse();
		if (leftExp->mValue.mType != TokenType::LITERAL || rightExp->mValue.mType != TokenType::LITERAL) {
			return;
		}

		if (mValue.mText == "-") {
			if (leftExp->mValue.mSubType == TokenSubType::STRING_LITERAL || rightExp->mValue.mSubType == TokenSubType::STRING_LITERAL) return;
			if (leftExp->mValue.mSubType == TokenSubType::INTEGER_LITERAL && rightExp->mValue.mSubType == TokenSubType::INTEGER_LITERAL) {
				int64_t left = std::stol(leftExp->mValue.mText);
				int64_t right = std::stol(rightExp->mValue.mText);

				mValue.mType = TokenType::LITERAL;
				mValue.mSubType = TokenSubType::INTEGER_LITERAL;
				mValue.mText = std::to_string(left - right);
			} else if (leftExp->mValue.mSubType == TokenSubType::FLOAT_LITERAL || rightExp->mValue.mSubType == TokenSubType::FLOAT_LITERAL) {
				double left = std::stod(leftExp->mValue.mText);
				double right = std::stod(rightExp->mValue.mText);

				mValue.mType = TokenType::LITERAL;
				mValue.mSubType = TokenSubType::FLOAT_LITERAL;
				mValue.mText = std::to_string(left - right);
			}
		} else if (mValue.mText == "+") {
			if (leftExp->mValue.mSubType == TokenSubType::STRING_LITERAL || rightExp->mValue.mSubType == TokenSubType::STRING_LITERAL) {
				mValue.mType = TokenType::LITERAL;
				mValue.mSubType = TokenSubType::STRING_LITERAL;
				mValue.mText = leftExp->mValue.mText + rightExp->mValue.mText;
			} else if (leftExp->mValue.mSubType == TokenSubType::INTEGER_LITERAL && rightExp->mValue.mSubType == TokenSubType::INTEGER_LITERAL) {
				int64_t left = std::stol(leftExp->mValue.mText);
				int64_t right = std::stol(rightExp->mValue.mText);

				mValue.mType = TokenType::LITERAL;
				mValue.mSubType = TokenSubType::INTEGER_LITERAL;
				mValue.mText = std::to_string(left + right);
			} else if (leftExp->mValue.mSubType == TokenSubType::FLOAT_LITERAL || rightExp->mValue.mSubType == TokenSubType::FLOAT_LITERAL) {
				double left = std::stod(leftExp->mValue.mText);
				double right = std::stod(rightExp->mValue.mText);

				mValue.mType = TokenType::LITERAL;
				mValue.mSubType = TokenSubType::FLOAT_LITERAL;
				mValue.mText = std::to_string(left + right);
			}
		} else if (mValue.mText == "/") {
			if (leftExp->mValue.mSubType == TokenSubType::STRING_LITERAL || rightExp->mValue.mSubType == TokenSubType::STRING_LITERAL) return;
			if (leftExp->mValue.mSubType == TokenSubType::INTEGER_LITERAL && rightExp->mValue.mSubType == TokenSubType::INTEGER_LITERAL) {
				int64_t left = std::stol(leftExp->mValue.mText);
				int64_t right = std::stol(rightExp->mValue.mText);

				mValue.mType = TokenType::LITERAL;
				mValue.mSubType = TokenSubType::INTEGER_LITERAL;
				mValue.mText = std::to_string(left / right);
			} else if (leftExp->mValue.mSubType == TokenSubType::INTEGER_LITERAL && rightExp->mValue.mSubType == TokenSubType::FLOAT_LITERAL) {
				int64_t left = std::stol(leftExp->mValue.mText);
				double right = std::stod(rightExp->mValue.mText);

				mValue.mType = TokenType::LITERAL;
				mValue.mSubType = TokenSubType::FLOAT_LITERAL;
				mValue.mText = std::to_string(left / right);
			} else if (leftExp->mValue.mSubType == TokenSubType::FLOAT_LITERAL && rightExp->mValue.mSubType == TokenSubType::INTEGER_LITERAL) {
				double left = std::stod(leftExp->mValue.mText);
				int64_t right = std::stol(rightExp->mValue.mText);

				mValue.mType = TokenType::LITERAL;
				mValue.mSubType = TokenSubType::INTEGER_LITERAL;
				mValue.mText = std::to_string(left / right);
			} else if (leftExp->mValue.mSubType == TokenSubType::FLOAT_LITERAL && rightExp->mValue.mSubType == TokenSubType::FLOAT_LITERAL) {
				double left = std::stod(leftExp->mValue.mText);
				double right = std::stod(rightExp->mValue.mText);

				mValue.mType = TokenType::LITERAL;
				mValue.mSubType = TokenSubType::FLOAT_LITERAL;
				mValue.mText = std::to_string(left / right);
			}
		} else if (mValue.mText == "*") {
			if (leftExp->mValue.mSubType == TokenSubType::STRING_LITERAL && rightExp->mValue.mSubType == TokenSubType::STRING_LITERAL) return;
			if (leftExp->mValue.mSubType == TokenSubType::STRING_LITERAL || rightExp->mValue.mSubType == TokenSubType::STRING_LITERAL) {
				if (!(leftExp->mValue.mSubType == TokenSubType::INTEGER_LITERAL || rightExp->mValue.mSubType == TokenSubType::INTEGER_LITERAL)) return;
				mValue.mType = TokenType::LITERAL;
				mValue.mSubType = TokenSubType::STRING_LITERAL;

				if (leftExp->mValue.mSubType == TokenSubType::INTEGER_LITERAL) {
					int64_t left = std::stol(leftExp->mValue.mText);
					mValue.mText = "";
					for (int i = 0; i < left; i++) {
						mValue.mText += rightExp->mValue.mText;
					}
				} else {
					int64_t right = std::stol(rightExp->mValue.mText);
					mValue.mText = "";
					for (int i = 0; i < right; i++) {
						mValue.mText += leftExp->mValue.mText;
					}
				}
			} else if (leftExp->mValue.mSubType == TokenSubType::INTEGER_LITERAL && rightExp->mValue.mSubType == TokenSubType::INTEGER_LITERAL) {
				int64_t left = std::stol(leftExp->mValue.mText);
				int64_t right = std::stol(rightExp->mValue.mText);

				mValue.mType = TokenType::LITERAL;
				mValue.mSubType = TokenSubType::INTEGER_LITERAL;
				mValue.mText = std::to_string(left * right);
			} else if (leftExp->mValue.mSubType == TokenSubType::FLOAT_LITERAL || rightExp->mValue.mSubType == TokenSubType::FLOAT_LITERAL) {
				double left = std::stod(leftExp->mValue.mText);
				double right = std::stod(rightExp->mValue.mText);

				mValue.mType = TokenType::LITERAL;
				mValue.mSubType = TokenSubType::FLOAT_LITERAL;
				mValue.mText = std::to_string(left * right);
			}
		} else if (mValue.mText == "%") {
			if (leftExp->mValue.mSubType == TokenSubType::STRING_LITERAL || rightExp->mValue.mSubType == TokenSubType::STRING_LITERAL) return;
			if (leftExp->mValue.mSubType == TokenSubType::FLOAT_LITERAL || rightExp->mValue.mSubType == TokenSubType::FLOAT_LITERAL) return;

			if (leftExp->mValue.mSubType == TokenSubType::INTEGER_LITERAL && rightExp->mValue.mSubType == TokenSubType::INTEGER_LITERAL) {
				int64_t left = std::stol(leftExp->mValue.mText);
				int64_t right = std::stol(rightExp->mValue.mText);

				mValue.mType = TokenType::LITERAL;
				mValue.mSubType = TokenSubType::INTEGER_LITERAL;
				mValue.mText = std::to_string(left % right);
			}
		}

		delete leftExp;
		mChildren[0] = nullptr;
		delete rightExp;
		mChildren[1] = nullptr;
	}


} // forest::parser
