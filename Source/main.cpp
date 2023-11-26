#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_set>
#include <filesystem>

#include "Tokeniser.hpp"
#include "Parser.hpp"
#include "ConfigParser.hpp"
#include "X86_64LinuxYasmCompiler.hpp"

using namespace forest::parser;
namespace fs = std::filesystem;

int main(int argc, char** argv) {
	// If the path is a directory, look for the config file
	fs::path filePath = argc == 1 || argc > 2 ? "../Examples/rule110.tree" : argv[1];
	bool useProject = false;
	if (fs::is_directory(filePath)) {
		// We are in a directory, parse config file
		for (const auto& entry : fs::directory_iterator(filePath)) {
			if (entry.path().filename() == "frstconfig") {
				filePath = entry.path();
				useProject = true;
				std::cout << "Direntry: " << entry.path() << std::endl;
			}
		}
		if (!useProject) {
			std::cout << "Calling Forest compile on a directory. Expected a file called `frstconfig` but found nothing";
			return -1;
		}
	}

	if (argc >= 2) {
		// Run with arguments
		std::cout << "Using file: " << filePath << std::endl;
	}

	std::ifstream file;
	file.open(filePath);
	std::stringstream buffer;
	buffer << file.rdbuf();
	file.close();

	std::string code = buffer.str();

	std::vector<Token> tokens = Tokeniser::parse(code, filePath);

	CompileContext ctx;
	if (useProject) {
		// Relative to filepath, read entrypoint
		ctx = CompileContext(tokens);
		tokens.clear();
		std::stringstream().swap(buffer);
		filePath = filePath.replace_filename(ctx.m_Configuration.m_Entrypoint);
		file.open(filePath);
		buffer << file.rdbuf();
		file.close();
		tokens = Tokeniser::parse(buffer.str(), filePath);
	}

	Parser parser;
	Programme p = parser.parse(tokens);
	bool found_main = false;

	for (auto& function : p.functions) {
		if (function.mName == "main") {
			found_main = true;
			break;
		}
	}
	if (!found_main) {
		std::cerr << "Expected a function called 'main' to be in file: " << filePath << std::endl;
		return 1;
	}
	fs::path originalPath = filePath;

	X86_64LinuxYasmCompiler compiler;
	compiler.compile(filePath, p, ctx);
	std::vector<Programme> programmes;
	std::unordered_set<fs::path> paths;
	programmes.push_back(p);
	fs::path buildPath = filePath.parent_path() / "build";
	paths.insert(buildPath / filePath.stem().replace_extension("o"));

	for (size_t i = 0; i < programmes.size(); i++) {
		const auto& programme = programmes.at(i);
		for (const auto& import : programme.imports) {
			fs::path tempPath = filePath.replace_filename(import.mPath);
			if (paths.contains(buildPath / tempPath.stem().replace_extension("o"))) continue;
			tokens.clear();
			std::stringstream().swap(buffer);
			file.open(tempPath);
			buffer << file.rdbuf();
			file.close();
			tokens = Tokeniser::parse(buffer.str(), tempPath);
			parser = Parser();
			Programme nextProgramme = parser.parse(tokens);
			programmes.push_back(nextProgramme);
			paths.insert(buildPath / tempPath.stem().replace_extension("o"));
			compiler.compile(tempPath, nextProgramme, ctx);
		}
	}

	std::stringstream linker;
	linker << "ld";
	if (ctx.m_Configuration.m_BuildType != BuildType::DEBUG)
		linker << " -s -z noseparate-code ";
	else
		linker << " -g";

	linker << " -o " << buildPath.string() << "/" << originalPath.stem().string() << " --dynamic-linker=/lib64/ld-linux-x86-64.so.2 ";
	for (const auto& path : paths) {
		linker << path << " ";
	}

	std::vector<std::string> dependencies;
	for (const auto& programme : programmes) {
		for (auto& dependency : programme.libDependencies) {
			linker << " -l" << dependency;
		}
	}
	std::system(linker.str().c_str());

	std::stringstream run_command;
	run_command << buildPath.string() << "/" << originalPath.stem().string();
	std::system(run_command.str().c_str());

	return 0;
}
