class c_instruct
{
public:
	std::string dest;
	std::string comp;
	std::string jump;
};

/* Stores contents of the input file 
 * to a vector of strings for each line
 *
 *  @file_name name of assembly file
 */
std::vector<std::string> store_file(const char* file_name);

/* Handles pre defined, goto symbols
 * and variables
 * 
 * @program stored assembly file
 */
void handle_symbols(std::vector<std::string>& program);

/* Parses C instruction into it's
 * dest, comp, and jump components
 *
 * @program stored assembly file
 */
c_instruct handle_c_instruct(std::string instruction);

// Converts .asm file name to .hack file name
std::string asm_to_hack(char* in);