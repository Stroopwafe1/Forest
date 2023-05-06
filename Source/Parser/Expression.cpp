#include "Expression.hpp"

namespace forest::parser {

	Expression::~Expression() {
		delete mLeft;
		delete mRight;
	}
} // forest::parser