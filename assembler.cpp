#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "parser.hpp"

int main()
{
	// store contents of file
	std::vector<std::string> program = store_file("mult.asm");

	// replace pre defined and goto symbols with binary values
	handle_symbols(program);


	// create output file
	std::ofstream output("output.txt");

	// copy contents to output file
	for (auto l : program)
	{
		l = l + "\n";
		const char* c = l.c_str();
		output.write(c, l.size());
	}

	return 0;
}