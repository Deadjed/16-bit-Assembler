#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <filesystem>
#include <string.h>

#include "parser.hpp"

std::vector<std::string> store_file(const char* file_name)
{
	// declare input file stream
	std::ifstream infile;

	// open assembly file
	infile.open(file_name);
	if (!infile)
	{
		std::cerr << "Unable to open file: " << file_name;
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
	
	// create predefined symbol table, initialize with pre defined values
	std::map<std::string, std::string> symbin{
		{"SCREEN", "16384"},	// Screen
		{"KBD", "24576"},		// Keyboard
		{"SP", "0"},
		{"LCL", "1"},
		{"ARG", "2"},
		{"THIS", "3"},
		{"THAT", "4"}
	};
	
	// map ram locations
	for (int i = 0; i < 16; i++)
	{
		std::string v = std::to_string(i);
		std::string s = 'R' + std::to_string(i);
		symbin.emplace(s, v);
	}
	
	// map goto symbols 
	for (int i = 0; i < program.size(); i++)
	{ 
		if (program.at(i).at(0) == '(')
		{
			std::string tempname;
			for (int x = 1; x < program.at(i).size() - 1; x++)
				tempname = tempname + program.at(i).at(x);
			std::string tempval = std::to_string(i);
			symbin.emplace(tempname, tempval);
			
			auto iter = program.begin();
			for (int x = i; x > 0; x--)
				iter++;
			program.erase(iter);
			i--;
		}
	}

	// replace A instructions with binary values
	int var = 16;
	for (int i = 0; i < program.size(); i++)
	{
		
		if (program.at(i).at(0) == '@')
		{
			program.at(i).erase(0, 1);

			for (auto& m : symbin)
			{
				if (program.at(i) == m.first)
					program.at(i) = '@' + m.second;
			}
			

			// if symbol not found replace with binary of variable counter
			// assuming variables can't start with @!
			if (program.at(i).at(0) != '@')
			{
				// if constant number used store that value instead
				if (isdigit(program.at(i).at(0)))
				{
					program.at(i) = '@' + program.at(i);
					continue;
				}

				std::string varval = std::to_string(var);
				std::string const varname = program.at(i);
				std::cout << varname << ": " << varval << "\n";
				symbin.emplace(varname, varval);
				std::cout << symbin[program.at(i)] << "\n";
				program.at(i) = '@' + varval;
				var++;
				
			}
		}
	}

	// check map values
	//std::cout << "Name \tLocation\n";
	//for (auto &i : symbin)
		//std::cout << i.first << ":\t" << i.second << std::endl;



	/*
	// Warn user of unused variables
	for (auto &s : symbin)
	{
		for (auto ptr = program.begin(); ptr < program.end(); ptr++)
		{
			if (*ptr == s.second)
				break;
			if (ptr == program.end())
				std::cerr << "WARNING! variable: " << s.first << " not used!" << std::endl;
		}
	}
	*/
}

// Store dest, comp, and jump in c_instruct.dest c_instruct.comp and c_instruct.jump
c_instruct handle_c_instruct(std::string instruction)
{
	c_instruct line;
	std::string temp;

	for (int i = 0; i < instruction.size(); i++)
	{
		// if theres a dest bit store that in line.dest
		if (instruction.at(i) == '=')
		{
			for (int x = 0; x < i; ++x)
				temp = temp + instruction.at(x);
			line.dest = temp;
			temp.clear();
			instruction.erase(0, i + 1);
			i = 0;

			// store remainder comp 
			for (int x = 0; x < instruction.size(); x++)
				temp = temp + instruction.at(x);
			line.comp = temp;
			temp.clear();
			i = 0;

			instruction.clear();
			break;
		}

		// store comp in line.dest
		if (instruction.at(i) == ';')
		{
			for (int x = 0; x < i; ++x)
				temp = temp + instruction.at(x);
			line.comp = temp;
			temp.clear();
			instruction.erase(0, i + 1);
			i = 0;
		}
	}
	if (line.dest.empty())
		line.dest = "null";
	if (instruction.empty())
		line.jump = "null";
	else
	{
		for (int x = 0; x < instruction.size(); x++)
			temp = temp + instruction.at(x);
		line.jump = temp;
	}

	return line;
}

std::string asm_to_hack(char* in)
{
	std::filesystem::path p = in;
	p.replace_extension(".hack");
	return p.string();
}