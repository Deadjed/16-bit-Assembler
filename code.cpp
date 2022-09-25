#include <vector>
#include <string>
#include <bitset>
#include <map>
#include <iostream>

#include "parser.hpp"


void binarize(std::vector<std::string>& program)
{
	std::map<std::string, std::string> compbin{
		{"0", "0101010"},
		{"1", "0111111"},
		{"-1", "0111010"},
		{"D", "0001100"},
		{"A", "0110000"},
		{"!D", "0001101"},
		{"!A", "0110001"},
		{"-D", "0001111"},
		{"-A", "0110011"},
		{"D+1", "0011111"},
		{"A+1", "0110111"},
		{"D-1", "0001110"},
		{"A-1", "0110010"},
		{"D+A", "0000010"},
		{"A+D", "0000010"},
		{"D-A", "0010011"},
		{"A-D", "0000111"},
		{"D&A", "0000000"},
		{"A&D", "0000000"},
		{"D|A", "0010101"},
		{"A|D", "0010101"},
		{"M", "1110000"},
		{"!M", "1110001"},
		{"-M", "1110011"},
		{"M+1", "1110111"},
		{"M-1", "1110010"},
		{"D+M", "1000010"},
		{"M+D", "1000010"},
		{"D-M", "1010011"},
		{"M-D", "1000111"},
		{"D&M", "1000000"},
		{"M&D", "1000000"},
		{"D|M", "1010101"},
		{"M|D", "1010101"}
	};

	std::map<std::string, std::string> destbin{
		{"null", "000"},
		{"M", "001"},
		{"D", "010"},
		{"MD", "011"},
		{"A", "100"},
		{"AM", "101"},
		{"AD", "110"},
		{"AMD", "111"}
	};

	std::map<std::string, std::string> jumpbin{
		{"null", "000"},
		{"JGT", "001"},
		{"JEQ", "010"},
		{"JGE", "011"},
		{"JLT", "100"},
		{"JNE", "101"},
		{"JLE", "110"},
		{"JMP", "111"}
	};

	for (int i = 0; i < program.size(); i++)
	{
		// replace A instructions with binary values
		if (program.at(i).at(0) == '@')
		{
			program.at(i).erase(0, 1);
			program.at(i) = '0' + std::bitset<15>(std::stol(program.at(i))).to_string();
		}
		else
		{
			c_instruct line = handle_c_instruct(program.at(i));
			//std::cout << "dest:" << line.dest << " \tcomp:" << line.comp << " \tjump:" << line.jump << std::endl;
			program.at(i) = "111" + compbin[line.comp] + destbin[line.dest] + jumpbin[line.jump];
			
		}
	}
}

// A INSTRUCTIONS (@VALUE)
// 0 + ValueInBinary (16 bits total)


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
