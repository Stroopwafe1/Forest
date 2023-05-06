#ifndef FOREST_EXPRESSION_HPP
#define FOREST_EXPRESSION_HPP

#include "Node.hpp"

namespace forest::parser {

	class Expression {
	public:
		Node* mRoot;

		~Expression();


	};

} // forest::parser

#endif //FOREST_EXPRESSION_HPP
