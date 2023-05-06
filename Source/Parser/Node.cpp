#include "Node.hpp"

namespace forest::parser {

	Node::~Node() {
		delete mLeft;
		delete mRight;
	}
} // forest::parser