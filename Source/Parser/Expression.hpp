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
		Expression();
		Expression(Expression const& rhs);
		void Collapse();
	};

} // forest::parser

#endif //FOREST_EXPRESSION_HPP
