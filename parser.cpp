// map all symbols 
	// first pass replace pre defined symbols (eg screen, keyboard)
	// second pass map labels (destinations of goto symbols)
	// third pass add variables (memory locations of variables)

// remove white space

// handle A and C instructions

// A INSTRUCTIONS (@VALUE)
// 0ValueInBinary (16 bits total)


// C INSTRUCTIONS
// 1 1 1 a c1 c2 c3 c4 c5 c6 d1 d2 d3 j1 j2 j3


/*
COMP		c1 c2 c3 c4 c5 c6
0			1  0  1  0  1  0
1			1  1  1  1  1  1
-1			1  1  1  0  1  0
D			0  0  1  1  0  0
A		M	1  1  0  0  0  0
!D			0  0  1  1  0  1
!A	   !M	1  1  0  0  0  1
-D			0  0  1  1  1  1
-A	   -M	1  1  0  0  1  1
D+1			0  1  1  1  1  1
A+1	   M+1	1  1  0  1  1  1
D-1			0  0  1  1  1  0
A-1	   M-1	1  1  0  0  1  0
D+A	   D+M	0  0  0  0  1  0
D-A	   D-M	0  1  0  0  1  1
A-D	   M-D	0  0  0  1  1  1
D&A	   D&M	0  0  0  0  0  0
D|A	   D|M	0  1  0  1  0  1

a=0		a=1
*/

/*
DEST	d1 d2 d3	effect: the value is stored in:
null	0  0  0		The value is not stored
M		0  0  1		RAM[A]
D		0  1  0		D register
MD		0  1  1		RAM[A] and D register
A		1  0  0		A register
AM		1  0  1		A register and RAM[A]
AD		1  1  0		A register and D register
AMD		1  1  1		A register, RAM[A], and D register
*/

/*
JUMP	j1 j2 j3	effect
null	0  0  0		no jump
JGT		0  0  1		if out > 0 jump
JEQ		0  1  0		if out = 0 jump
JGE		0  1  1		if out >= 0 jump
JLT		1  0  0		if out < 0 jump
JNE		1  0  1		if out != 0 jump
JLE		1  1  0		if out <= 0 jump
JMP		1  1  1		Unconditional jump
*/

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <bitset>

std::vector<std::string> store_file(const char* file_name)
{
	// declare input file stream
	std::ifstream infile;

	// open assembly file
	infile.open(file_name);
	if (!infile)
	{
		std::cerr << "Unable to open file: " << "Mult.asm";
		exit(1);
	}

	// store file into a vector
	std::vector<std::string> contents;
	std::string line;

	while (infile)
	{
		char c = NULL;
		line.clear();

		while (infile.get(c))
		{
			// remove comments
			if (c == '/')
			{
				while (c != '\n')
					infile.get(c);
			}

			// store each line as seperate string within vector
			if (c == '\n') 
				break;

			// only store non white space characters
			if (c != ' ')
				line = line + c;
		}
		// don't store empty lines
		if (!line.empty())
			contents.push_back(line);
	}

	infile.close();

	return contents;
}


void handle_symbols(std::vector<std::string>& program)
{
	// create predefined symbol table
	std::map<std::string, std::string> symbin;

	// map ram locations
	for (int i = 0; i < 16; i++)
	{
		std::string n = '0' + std::bitset<15>(i).to_string();
		std::string s = "R" + std::to_string(i);
		symbin.emplace(s, n);
	}

	// map goto symbols 
	for (int i = 0; i < program.size(); i++)
	{
		if (program.at(i).at(0) == '(')
		{
			std::string tempname;
			for (int x = 1; x < program.at(i).size() - 1; x++)
				tempname = tempname + program.at(i).at(x);
			std::string tempval = '0' + std::bitset<15>(i + 2).to_string();
			symbin.emplace(tempname, tempval);
		}
	}

	// check map values
	for (auto i : symbin)
		std::cout << i.first << ": " << i.second << std::endl;
}