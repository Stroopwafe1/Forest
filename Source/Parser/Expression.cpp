#include "Expression.hpp"

namespace forest::parser {

	Expression::~Expression() {
		delete mLeft;
		delete mRight;
	}

	void Expression::Collapse() {
		if (mLeft == nullptr && mRight == nullptr) {
			return;
		}
		if (mValue.mType != TokenType::OPERATOR) {
			return;
		}

		mLeft->Collapse();
		mRight->Collapse();
		if (mLeft->mValue.mType != TokenType::LITERAL || mRight->mValue.mType != TokenType::LITERAL) {
			return;
		}

		if (mValue.mText == "-") {
			if (mLeft->mValue.mSubType == TokenSubType::STRING_LITERAL || mRight->mValue.mSubType == TokenSubType::STRING_LITERAL) return;
			if (mLeft->mValue.mSubType == TokenSubType::INTEGER_LITERAL && mRight->mValue.mSubType == TokenSubType::INTEGER_LITERAL) {
				int64_t left = std::stol(mLeft->mValue.mText);
				int64_t right = std::stol(mRight->mValue.mText);

				mValue.mType = TokenType::LITERAL;
				mValue.mSubType = TokenSubType::INTEGER_LITERAL;
				mValue.mText = std::to_string(left - right);
			} else if (mLeft->mValue.mSubType == TokenSubType::FLOAT_LITERAL || mRight->mValue.mSubType == TokenSubType::FLOAT_LITERAL) {
				double left = std::stod(mLeft->mValue.mText);
				double right = std::stod(mRight->mValue.mText);

				mValue.mType = TokenType::LITERAL;
				mValue.mSubType = TokenSubType::FLOAT_LITERAL;
				mValue.mText = std::to_string(left - right);
			}
		} else if (mValue.mText == "+") {
			if (mLeft->mValue.mSubType == TokenSubType::STRING_LITERAL || mRight->mValue.mSubType == TokenSubType::STRING_LITERAL) {
				mValue.mType = TokenType::LITERAL;
				mValue.mSubType = TokenSubType::STRING_LITERAL;
				mValue.mText = mLeft->mValue.mText + mRight->mValue.mText;
			} else if (mLeft->mValue.mSubType == TokenSubType::INTEGER_LITERAL && mRight->mValue.mSubType == TokenSubType::INTEGER_LITERAL) {
				int64_t left = std::stol(mLeft->mValue.mText);
				int64_t right = std::stol(mRight->mValue.mText);

				mValue.mType = TokenType::LITERAL;
				mValue.mSubType = TokenSubType::INTEGER_LITERAL;
				mValue.mText = std::to_string(left + right);
			} else if (mLeft->mValue.mSubType == TokenSubType::FLOAT_LITERAL || mRight->mValue.mSubType == TokenSubType::FLOAT_LITERAL) {
				double left = std::stod(mLeft->mValue.mText);
				double right = std::stod(mRight->mValue.mText);

				mValue.mType = TokenType::LITERAL;
				mValue.mSubType = TokenSubType::FLOAT_LITERAL;
				mValue.mText = std::to_string(left + right);
			}
		} else if (mValue.mText == "/") {
			if (mLeft->mValue.mSubType == TokenSubType::STRING_LITERAL || mRight->mValue.mSubType == TokenSubType::STRING_LITERAL) return;
			if (mLeft->mValue.mSubType == TokenSubType::INTEGER_LITERAL && mRight->mValue.mSubType == TokenSubType::INTEGER_LITERAL) {
				int64_t left = std::stol(mLeft->mValue.mText);
				int64_t right = std::stol(mRight->mValue.mText);

				mValue.mType = TokenType::LITERAL;
				mValue.mSubType = TokenSubType::INTEGER_LITERAL;
				mValue.mText = std::to_string(left / right);
			} else if (mLeft->mValue.mSubType == TokenSubType::INTEGER_LITERAL && mRight->mValue.mSubType == TokenSubType::FLOAT_LITERAL) {
				int64_t left = std::stol(mLeft->mValue.mText);
				double right = std::stod(mRight->mValue.mText);

				mValue.mType = TokenType::LITERAL;
				mValue.mSubType = TokenSubType::FLOAT_LITERAL;
				mValue.mText = std::to_string(left / right);
			} else if (mLeft->mValue.mSubType == TokenSubType::FLOAT_LITERAL && mRight->mValue.mSubType == TokenSubType::INTEGER_LITERAL) {
				double left = std::stod(mLeft->mValue.mText);
				int64_t right = std::stol(mRight->mValue.mText);

				mValue.mType = TokenType::LITERAL;
				mValue.mSubType = TokenSubType::INTEGER_LITERAL;
				mValue.mText = std::to_string(left / right);
			} else if (mLeft->mValue.mSubType == TokenSubType::FLOAT_LITERAL && mRight->mValue.mSubType == TokenSubType::FLOAT_LITERAL) {
				double left = std::stod(mLeft->mValue.mText);
				double right = std::stod(mRight->mValue.mText);

				mValue.mType = TokenType::LITERAL;
				mValue.mSubType = TokenSubType::FLOAT_LITERAL;
				mValue.mText = std::to_string(left / right);
			}
		} else if (mValue.mText == "*") {
			if (mLeft->mValue.mSubType == TokenSubType::STRING_LITERAL && mRight->mValue.mSubType == TokenSubType::STRING_LITERAL) return;
			if (mLeft->mValue.mSubType == TokenSubType::STRING_LITERAL || mRight->mValue.mSubType == TokenSubType::STRING_LITERAL) {
				if (!(mLeft->mValue.mSubType == TokenSubType::INTEGER_LITERAL || mRight->mValue.mSubType == TokenSubType::INTEGER_LITERAL)) return;
				mValue.mType = TokenType::LITERAL;
				mValue.mSubType = TokenSubType::STRING_LITERAL;

				if (mLeft->mValue.mSubType == TokenSubType::INTEGER_LITERAL) {
					int64_t left = std::stol(mLeft->mValue.mText);
					mValue.mText = "";
					for (int i = 0; i < left; i++) {
						mValue.mText += mRight->mValue.mText;
					}
				} else {
					int64_t right = std::stol(mRight->mValue.mText);
					mValue.mText = "";
					for (int i = 0; i < right; i++) {
						mValue.mText += mLeft->mValue.mText;
					}
				}
			} else if (mLeft->mValue.mSubType == TokenSubType::INTEGER_LITERAL && mRight->mValue.mSubType == TokenSubType::INTEGER_LITERAL) {
				int64_t left = std::stol(mLeft->mValue.mText);
				int64_t right = std::stol(mRight->mValue.mText);

				mValue.mType = TokenType::LITERAL;
				mValue.mSubType = TokenSubType::INTEGER_LITERAL;
				mValue.mText = std::to_string(left * right);
			} else if (mLeft->mValue.mSubType == TokenSubType::FLOAT_LITERAL || mRight->mValue.mSubType == TokenSubType::FLOAT_LITERAL) {
				double left = std::stod(mLeft->mValue.mText);
				double right = std::stod(mRight->mValue.mText);

				mValue.mType = TokenType::LITERAL;
				mValue.mSubType = TokenSubType::FLOAT_LITERAL;
				mValue.mText = std::to_string(left * right);
			}
		} else if (mValue.mText == "%") {
			if (mLeft->mValue.mSubType == TokenSubType::STRING_LITERAL || mRight->mValue.mSubType == TokenSubType::STRING_LITERAL) return;
			if (mLeft->mValue.mSubType == TokenSubType::FLOAT_LITERAL || mRight->mValue.mSubType == TokenSubType::FLOAT_LITERAL) return;

			if (mLeft->mValue.mSubType == TokenSubType::INTEGER_LITERAL && mRight->mValue.mSubType == TokenSubType::INTEGER_LITERAL) {
				int64_t left = std::stol(mLeft->mValue.mText);
				int64_t right = std::stol(mRight->mValue.mText);

				mValue.mType = TokenType::LITERAL;
				mValue.mSubType = TokenSubType::INTEGER_LITERAL;
				mValue.mText = std::to_string(left % right);
			}
		}

		if (mLeft != nullptr) {
			delete mLeft;
			mLeft = nullptr;
		}
		if (mRight != nullptr) {
			delete mRight;
			mRight = nullptr;
		}
	}
} // forest::parser