#ifndef GENERATOR_H
#define GENERATOR_H

#include <iostream>
#include "automaton-data.hpp"

void generateAutomaton(std::ostream& out, const Automaton& automaton);

#endif // GENERATOR_H