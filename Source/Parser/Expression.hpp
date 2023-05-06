#ifndef FOREST_EXPRESSION_HPP
#define FOREST_EXPRESSION_HPP

#include "Tokeniser.hpp"

namespace forest::parser {

	class Expression {
	public:
		Expression* mLeft = nullptr;
		Expression* mRight = nullptr;
		Token mValue {};

		~Expression();
		void Collapse();
	};

} // forest::parser

#endif //FOREST_EXPRESSION_HPP
