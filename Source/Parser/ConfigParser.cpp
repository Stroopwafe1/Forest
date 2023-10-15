#include <iostream>
#include <sstream>
#include "ConfigParser.hpp"

namespace forest::parser {
	ConventionPattern::ConventionPattern(const std::string& pattern) {
		// $symbolName $symbol_name $SymbolName $SYMBOL_NAME
		if (pattern == "") return;
		bool parsedPrefix = false;
		bool parsedCasing = false;
		int startUpper = -1;
		bool foundUnderscore = false;
		for (char c : pattern) {
			if (!parsedPrefix) {
				if (c != '$')
					prefix += c;
				else
					parsedPrefix = true;
				continue;
			}
			if (!parsedCasing) {
				// Last char is always e or E
				if (c == 'e' || c == 'E')
					parsedCasing = true;
				if (c == 'S') {
					// Cannot be snake_case or camelCase anymore
					startUpper = 1;
				} else if (c == 's') {
					// Cannot be SCREAMING_SNAKE_CASE or PascalCase anymore
					startUpper = 0;
				}
				if (c == '_') {
					foundUnderscore = true;
				}
				continue;
			}
			suffix += c;
		}
		if (startUpper == 0) {
			if (foundUnderscore) casing = Casing::SNAKE_CASE;
			else casing = Casing::CAMEL_CASE;
		} else if (startUpper == 1) {
			if (foundUnderscore) casing = Casing::SCREAMING_SNAKE_CASE;
			else casing = Casing::PASCAL_CASE;
		} else {
			std::cerr << "Unexpected value found while parsing casing pattern" << std::endl;
			return;
		}
	}

	CompileContext::CompileContext(std::vector<Token>& tokens) {
		_conventions = {};
		_currentToken = tokens.begin();
		_tokensEnd = tokens.end();
		while (_currentToken != tokens.end()) {
			if (_currentToken->mText == "[") {
				_currentHeader = expectHeader();
			}
			switch (_currentHeader) {
				case HeaderType::CONFIGURATION: {
					std::optional<Token> configTypeOpt = expectIdentifier();
					if (!configTypeOpt.has_value()) {
						std::cerr << "Expected a configuration type (BuildType, Entrypoint, etc...) at " << *_currentToken << std::endl;
						return;
					}
					std::optional<Token> equals = expectOperator("=");
					if (!equals.has_value()) {
						std::cerr << "Expected an '=' after the configuration type at " << *_currentToken << std::endl;
						return;
					}

					if (configTypeOpt.value().mText == "BuildType") {
						std::optional<Token> type = expectIdentifier();
						if (!type.has_value()) {
							std::cerr << "Expected a build type (Debug, Release, etc...) at " << *_currentToken << std::endl;
							return;
						}
						m_Configuration.m_BuildType = getTypeFromConfig(type.value().mText);
					} else if (configTypeOpt.value().mText == "Entrypoint") {
						std::stringstream ss;
						while (_currentToken->mText != "tree") {
							ss << _currentToken->mText;
							_currentToken++;
						}
						ss << _currentToken->mText;
						_currentToken++;
						m_Configuration.m_Entrypoint = ss.str();
					}
					break;
				}
				case HeaderType::CONVENTIONS: {
					std::optional<ConventionEntry> convention = expectConvention();
					if (!convention.has_value()) {
						std::cerr << "Expected to be able to parse a convention" << std::endl;
						return;
					}
					_conventions.push_back(convention.value());
					break;
				}
				case HeaderType::NONE:
					break;
			}
		}
		conventionsEnd = _conventions.cend();
	}

	const std::vector<ConventionEntry>::const_iterator CompileContext::getSymbolConvention(const std::string& name) const {
		for (std::vector<ConventionEntry>::const_iterator convention = _conventions.cbegin(); convention < _conventions.cend(); convention++) {
			const ConventionEntry& con = *convention;
			bool prefixIsSame = name.substr(0, con.pattern.prefix.length()) == con.pattern.prefix;
			bool suffixIsSame = name.substr(name.length() - con.pattern.suffix.length()) == con.pattern.suffix;
			bool impossible = false;
			if (!prefixIsSame || !suffixIsSame) continue;
			std::string actualName = name.substr(con.pattern.prefix.length(), name.length() - con.pattern.suffix.length() - con.pattern.prefix.length());
			int prevCase = -1; // 0 for lowercase, 1 for uppercase
			for (char c : actualName) {
				if (c == '_') {
					if (prevCase == 0 && con.pattern.casing == Casing::SNAKE_CASE) return convention;
					else if (prevCase == 1 && con.pattern.casing == Casing::SCREAMING_SNAKE_CASE) return convention;
					else {
						impossible = true;
						break;
					}
				}
				int currCase = c >= 'A' && c <= 'Z';
				if (currCase == prevCase) continue;
				if (prevCase == 0 && con.pattern.casing == Casing::CAMEL_CASE) return convention; // Previously lowercase, now uppercase
				else if (prevCase == 1 && con.pattern.casing == Casing::PASCAL_CASE) return convention; // Previously uppercase, now lowercase
				prevCase = currCase;
			}
			if (impossible) continue;
			// camelCase and snake_case are the same if no 2nd word is defined, so we might have two options for case detection if it's all been lowercase and we didn't find a '_'
			if (prevCase == 0 && (con.pattern.casing == Casing::CAMEL_CASE || con.pattern.casing == Casing::SNAKE_CASE)) return convention;
			else continue;
		}
		return _conventions.cend();
	}

	std::optional<Token> CompileContext::expectIdentifier(const std::string& name) {
		if (_currentToken->mType != TokenType::IDENTIFIER) return std::nullopt;
		if (!name.empty() && _currentToken->mText != name) return std::nullopt;

		return *_currentToken++;
	}

	std::optional<Token> CompileContext::expectOperator(const std::string& name) {
		if (_currentToken->mType != TokenType::OPERATOR) return std::nullopt;
		if (!name.empty() && _currentToken->mText != name) return std::nullopt;

		return *_currentToken++;
	}

	HeaderType CompileContext::expectHeader() {
		HeaderType returnVal = HeaderType::NONE;
		std::optional<Token> openingBracket = expectOperator("[");
		std::optional<Token> header = expectIdentifier();
		if (!header.has_value()) {
			std::cerr << "Expected a header name after the `[` at " << openingBracket.value() << std::endl;
			return HeaderType::NONE;
		}

		if (header.value().mText == "Configuration") returnVal = HeaderType::CONFIGURATION;
		else if (header.value().mText == "Conventions") returnVal = HeaderType::CONVENTIONS;
		else {
			std::cerr << "Unexpected header type in frstconfig `" << header.value().mText << "` at " << header.value() << std::endl;
			return HeaderType::NONE;
		}

		std::optional<Token> closingBracket = expectOperator("]");
		if (!closingBracket.has_value()) {
			std::cerr << "Expected a closing `]` for the `[` at " << openingBracket.value() << std::endl;
			return HeaderType::NONE;
		}

		return returnVal;
	}

	std::optional<ConventionEntry> CompileContext::expectConvention() {
		std::optional<Token> visibilityOpt = expectIdentifier();
		if (!visibilityOpt.has_value()) {
			std::cerr << "Expected a visibility modifier (pub, pri, pro) in the convention parsing at " << *_currentToken << std::endl;
			return std::nullopt;
		}
		ConventionEntry returnVal;

		Token& visibility = visibilityOpt.value();
		if (visibility.mText == "pub") returnVal.modifiers |= Modifiers::PUBLIC;
		else if (visibility.mText == "pri") returnVal.modifiers |= Modifiers::PRIVATE;
		else if (visibility.mText == "pro") returnVal.modifiers |= Modifiers::PROTECTED;
		else {
			std::cerr << "Unexpected visibility modifier `" << visibility.mText << "`. Expected one of (pub, pri, pro) at " << visibility << std::endl;
			return std::nullopt;
		}

		while (_currentToken->mText != "=") {
			std::optional<Token> modOrTypeOpt = expectIdentifier();
			if (!modOrTypeOpt.has_value()) {
				// We have neither identifier or =, so something's gone wrong
				std::cerr << "Expected modifier or symbol type while parsing conventions at " << *_currentToken << std::endl;
				return std::nullopt;
			}
			Token& modOrType = modOrTypeOpt.value();
			if (modOrType.mText == "fn") returnVal.symbolType = SymbolType::FUNCTION;
			else if (modOrType.mText == "var") returnVal.symbolType = SymbolType::VARIABLE;
			else if (modOrType.mText == "member") returnVal.symbolType = SymbolType::MEMBER;
			else if (modOrType.mText == "struct") returnVal.symbolType = SymbolType::STRUCT;
			else if (modOrType.mText == "class") returnVal.symbolType = SymbolType::CLASS;
			else if (modOrType.mText == "interface") returnVal.symbolType = SymbolType::INTERFACE;
			else if (modOrType.mText == "enum") returnVal.symbolType = SymbolType::ENUM;

			else if (modOrType.mText == "const") returnVal.modifiers |= Modifiers::CONSTANT;
			else if (modOrType.mText == "static") returnVal.modifiers |= Modifiers::STATIC;
			else if (modOrType.mText == "global") returnVal.modifiers |= Modifiers::GLOBAL;
			else {
				std::cerr << "Unexpected modifier or symbol type `" << modOrType.mText << "` while parsing conventions at " << modOrType << std::endl;
				return std::nullopt;
			}
		}

		expectOperator("=");

		std::optional<Token> patternOpt = expectIdentifier();
		if (!patternOpt.has_value()) {
			std::cerr << "Expected a pattern while parsing conventions at " << *_currentToken << std::endl;
			return std::nullopt;
		}
		returnVal.pattern = patternOpt.value().mText;

		return returnVal;
	}

	BuildType CompileContext::getTypeFromConfig(const std::string& config) {
		if (config == "Debug") {
			return BuildType::DEBUG;
		} else if (config == "Release") {
			return BuildType::RELEASE;
		} else {
			return BuildType::DEBUG;
		}
	}
}
