# 16 bit Assembler
The assembler takes a file written in hack assembly code and converts it to 16 bit binary instructions with a .hack extension.   
Usage: ```Prompt>SIT111_16_bit_assembler.exe FILENAME.asm```

## Main Function
The main function of this program is quite simple. It first **stores** the assembly programs contents in memory, removing all white space and comments in the process. Then **replaces all symbols** with their values, removing labels in that process. Finally it **converts** each line into it's binary equivelent before writing it all to an output file.   

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
Of course the code is more in depth than that, first lets look at how the program is *storing* the file while removing comments and white space. The assembly program itself is stored line by line in a **vector of strings**. This is achieved by opening the file as an f stream, then iterating over each line, then iterating over each character in that line. If that character is not white space it is stored, unless it is a '/' character implying it is a comment and the rest of the line is then ignored until the '\n' character which when hit all stored characters are stores as a line (in a string format) within the vector holding this assembly program. Keeping in mind that only checking of one '/' character is not perfect when two in a row are used to notate a comment, also /* */ notation is completely ignored in this method (assuming those comments exist in this assembly even though I have not seen them).   
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
To parse symbols within the assembly code a map must be made to hold name/value pairs of each symbol and their represented value. I have stored these values as a **map<string, string>** type. Initially filling it with pre defined values such as RAM locations (0-15), the Screen, keyboard etc.. Then iterating over each line storing label names, and the line the iterator is up to, and then removing the label from the code.  
Once all pre defined symbols and labels are mapped, itterating over each line of code searching for **A instructions** again we can assume that all unknown symbols are now *variables*, and their name is stored in the map along with another iterator starting at 16 (location of first variable).
```cpp
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
}
```
## Parsing C Instructions
To parse the **C instructions** I created a class to hold each component of the instruction (Destination, Compare, amd Jump) seperately as a string. Due to the order **Dest = Comp ; Jump**, I decided to iterate over each character of the instruction searching for an '=' character. If found all previous characters will be stored in the classes dest string, then all following characters will be stored in the comp string leaving the jump string null.   
While searching for the '=' character, we are also searching for the ';' character, if this one is found then all characters before it are stored in the classes comp string, and all following characters are stored in the classes jump string leaving the dest string null.   

```cpp
class c_instruct
{
public:
	std::string dest;
	std::string comp;
	std::string jump;
};

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
To convert each instruction to binary is a lot easier once all the parsing has been done. for **A instructions** it is as simple as setting the first bit to '0' and then converting the @ instructions value to a binary value for the following 15 bits to make up it's 16 bit instruction. If the line is not an **A instruction** then it is a **C instruction** and is sent to the parser to seperate each component (*mentioned above*). The **C instruction** is stored as such ```1 1 1 a c1 c2 c3 c4 c5 c6 d1 d2 d3 j1 j2 j3``` with the c bits representing **comp** bits, d bits representing **dest** bits, and j bits representing **jump** bits. To convert these components to their binary values another symbol table must be made, here I have mapped all of their values to their binary equivelents and stored each line to match the above format via ```program.at(i) = "111" + compbin[line.comp] + destbin[line.dest] + jumpbin[line.jump];``` which access each binary value in the map using the seperated components the parser has supplied us with.

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
```
## Output File
Output files keep the same name just replacing their **.asm** file extension to a **.hack** one.
```cpp
std::string asm_to_hack(char* in)
{
	std::filesystem::path p = in;
	p.replace_extension(".hack");
	return p.string();
}
```