#ifndef FOREST_NODE_HPP
#define FOREST_NODE_HPP

#include "Tokeniser.hpp"

namespace forest::parser {

	class Node {
	public:
		Node* mLeft = nullptr;
		Node* mRight = nullptr;
		Token mValue;

		~Node();
	};

} // forest::parser

#endif //FOREST_NODE_HPP
