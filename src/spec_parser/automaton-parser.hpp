/**
 * @brief Parse a saved file to an automaton instance
 * @author Jakub Kovarik
 */
#ifndef AUTOMATON_PARSER_H
#define AUTOMATON_PARSER_H

#include "automaton-data.hpp"

class AutomatonParser
{
public:
    // expose a single static parsing method
    static void FromFile(std::string const filename, Automaton& outAutomaton);
};

#endif