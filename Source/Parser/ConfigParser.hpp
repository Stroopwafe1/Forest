#ifndef FOREST_CONFIGPARSER_HPP
#define FOREST_CONFIGPARSER_HPP

#include <string>
#include <vector>
#include <optional>
#include "Tokeniser.hpp"

namespace forest::parser {
	enum class Modifiers : int {
		PUBLIC    = 1 << 0,
		PRIVATE   = 1 << 1,
		PROTECTED = 1 << 2,
		CONSTANT  = 1 << 3,
		STATIC    = 1 << 4,
		GLOBAL    = 1 << 5,
	};

	inline Modifiers operator|(Modifiers lhs, Modifiers rhs) {
		return static_cast<Modifiers>(static_cast<int>(lhs) | static_cast<int>(rhs));
	}
	inline Modifiers operator&(Modifiers lhs, Modifiers rhs) {
		return static_cast<Modifiers>(static_cast<int>(lhs) & static_cast<int>(rhs));
	}
	inline Modifiers operator|=(Modifiers& lhs, Modifiers rhs) {
		return lhs = lhs | rhs;
	}
	inline Modifiers operator&=(Modifiers& lhs, Modifiers rhs) {
		return lhs = lhs & rhs;
	}

	enum class SymbolType : int {
		VARIABLE = 1,
		MEMBER,
		FUNCTION,
		STRUCT,
		ENUM,
		CLASS,
		INTERFACE,
	};

	enum class Casing {
		CAMEL_CASE = 1,
		PASCAL_CASE,
		SNAKE_CASE,
		SCREAMING_SNAKE_CASE
	};

	struct ConventionPattern {
		std::string prefix{};
		std::string suffix{};
		Casing casing{};
		ConventionPattern(const std::string& pattern);
	};

	struct ConventionEntry {
		Modifiers modifiers{};
		SymbolType symbolType{};
		ConventionPattern pattern{""};
	};

	enum class HeaderType {
		NONE,
		CONFIGURATION,
		CONVENTIONS,
	};

	enum class BuildType {
		DEBUG,
		RELEASE
	};

	struct Configuration {
		std::string m_Entrypoint{};
		BuildType m_BuildType{};
	};

	class CompileContext {
	public:
		Configuration m_Configuration{};
		std::vector<ConventionEntry>::const_iterator conventionsEnd{};

		CompileContext() = default;
		CompileContext(std::vector<Token>& tokens);
		const std::vector<ConventionEntry>::const_iterator getSymbolConvention(const std::string& name) const;
	private:
		std::vector<ConventionEntry> _conventions;
		HeaderType _currentHeader{};
		std::vector<Token>::iterator _currentToken;
		std::vector<Token>::iterator _tokensEnd;

		std::optional<Token> expectIdentifier(const std::string& name = "");
		std::optional<Token> expectOperator(const std::string& name = "");
		HeaderType expectHeader();
		std::optional<ConventionEntry> expectConvention();
		BuildType getTypeFromConfig(const std::string& config);
	};
}

#endif
