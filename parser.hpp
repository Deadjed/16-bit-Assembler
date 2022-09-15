/* Stores contents of the input file 
 * to a vector of strings for each line
 *
 *  @file_name name of assembly file
 */
std::vector<std::string> store_file(const char* file_name);

/* Handles pre defined and goto symbols
 * 
 * @program stored assembly file
 */
void handle_symbols(std::vector<std::string>& program);