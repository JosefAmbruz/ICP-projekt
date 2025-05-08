#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <string>
#include "automaton-data.hpp"

void parseAutomaton(std::istream& in, Automaton& automaton);
bool startsWith(const std::string& str, const std::string& prefix);
std::string trim(const std::string& str);

#endif // PARSER_H