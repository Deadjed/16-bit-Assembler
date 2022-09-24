#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>

#include "parser.hpp"
#include "code.hpp"

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cerr << "Please insert file name!" << std::endl;
		return 1;
	}

	// store contents of file
	std::vector<std::string> program = store_file(argv[1]);

	// replace pre defined/goto symbols and variables with integer values
	handle_symbols(program);

	// convert assembly to binary
	binarize(program);

	// create output file
	auto out_file = asm_to_hack(argv[1]);
	std::ofstream output(out_file);
	
	// copy contents to output file
	for (auto &line : program)
	{
		line = line + "\n";
		const char* c = line.c_str();
		output.write(c, line.size());
	}
	
	return 0;
}