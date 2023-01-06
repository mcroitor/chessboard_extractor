#ifndef _FUNCTIONS_H_
#define _FUNCTIONS_H_

#include <map>
#include <string>

typedef std::map<std::string, std::string> keymap;

void build_keys(keymap &keys, int argc, char **argv);
void print_options(const keymap& options);
std::string ltrim(std::string str);
std::string rtrim(std::string str);
std::string trim(std::string str);

#endif
