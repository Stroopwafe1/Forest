#include "Expression.hpp"

namespace forest::parser {
	Expression::~Expression() {
		delete mRoot;
	}
} // forest::parser