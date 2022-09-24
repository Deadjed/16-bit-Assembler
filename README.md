# 16 bit Assembler
The assembler takes a file written in hack assembly code and converts it to 16 bit binary instructions with a .hack extension.

## Main Function
The main function of this program is quite simple. It first stores the assembly programs contents in memory, removing all white space and comments in the process. Then replaces all symbols with their values, removing labels in that process. Finally it converts each line into it's binary equivelent before writing it all to an output file.   

```cpp
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
```

## Storing The File
Of course the code is more in depth than that, first lets look at how the program is storing the file while removing comments and white space. The assembly program itself is stored line by line in a vector of strings. This is achieved by opening the file as an f stream, then iterating over each line, then iterating over each character in that line. If that character is not white space it is stored, unless it is a '/' character implying it is a comment and the rest of the line is then ignored until the '\n' character which when hit all stored characters are stores as a line (in a string format) within the vector holding this assembly program. Keeping in mind that only checking of one '/' character is not perfect when two in a row are used to notate a comment, also /* */ notation is completely ignored in this method (assuming those comments exist in this assembly even though I have not seen them).   
Once this function reaches the end of file the file is closed and the vector containing the program is returned back to the main function to then be parsed.

```cpp
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
```
## Parsing Symbols

```cpp
void handle_symbols(std::vector<std::string>& program)
{
	
	// create predefined symbol table
	std::map<std::string, std::string> symbin;
	
	// map ram locations
	for (int i = 0; i < 16; i++)
	{
		std::string v = std::to_string(i);
		std::string s = 'R' + std::to_string(i);
		symbin.emplace(s, v);
	}
	
	// map other pre defined locations
	std::string tmpname = "SCREEN";
	std::string tmpval = std::to_string(16384);
	symbin.emplace(tmpname, tmpval);
	
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
			// assuming variables can't start with 0!
			if (program.at(i).at(0) != '@')
			{
				std::string tmp = std::to_string(var);
				symbin.emplace(program.at(i), tmp);
				program.at(i) = '@' + tmp;
				var++;
			}
		}
	}
```
## Parsing C Instructions


```cpp
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
```
## Converting To Binary

```cpp
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
			program.at(i) = '0' + std::bitset<15>(std::stoi(program.at(i))).to_string();
		}
		else
		{
			c_instruct line = handle_c_instruct(program.at(i));
			//std::cout << "dest:" << line.dest << " \tcomp:" << line.comp << " \tjump:" << line.jump << std::endl;
			program.at(i) = "111" + compbin[line.comp] + destbin[line.dest] + jumpbin[line.jump];
			
		}
	}
}

std::string asm_to_hack(char* in)
{
	std::filesystem::path p = in;
	p.replace_extension(".hack");
	return p.string();
}