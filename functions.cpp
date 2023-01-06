#include <iostream>
#include "functions.h"

/**
 * helper function. remove leading spaces/tabs from string
 * @param str
 * @return
 */
std::string ltrim(std::string str)
{
    while (str.front() == ' ' || str.front() == '\t')
    {
        str.erase(str.begin());
    }
    return str;
}

/**
 * helper function. remove spaces/tabs from end of string
 * @param str
 * @return
 */
std::string rtrim(std::string str)
{
    while (str.back() == ' ' || str.back() == '\t')
    {
        str.pop_back();
    }
    return str;
}

/**
 * helper function. remove spaces/tabs from begin and end of string
 * @param str
 * @return
 */
std::string trim(std::string str)
{
    return ltrim(rtrim(str));
}

/**
 * @brief
 *
 * @param keys
 * @param argc
 * @param argv
 */
void build_keys(keymap &keys, int argc, char **argv)
{
    int i, tmp;
    std::string key, value, aux;
    for (i = 0; i != argc; ++i)
    {
        aux = std::string(argv[i]);
        tmp = aux.find('=');
        if (tmp == aux.npos)
        {
            continue;
        }
        key = aux.substr(2, tmp - 2);
        value = aux.substr(tmp + 1, aux.size() - tmp - 1);
        // std::cout << "[debug] key = " << key << ", value = " << value << std::endl;
        if (keys.count(key) == 0)
        {
            continue;
        }
        keys[key] = value;
    }
}

void print_options(const keymap& options){
    std::cout << "configuration: " << std::endl;
    for(auto option: options){
        std::cout << "\t" + option.first << "\t = " << option.second << std::endl;
    }
}