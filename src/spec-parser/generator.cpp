/**
 * @brief Generuje textovou specifikaci automatu
 * @todo otestovat
 */

#include "generator.hpp"
#include "automaton-data.hpp"

void generateAutomaton(std::ostream& out, const Automaton& automaton) {
    out << "AUTOMATON " << automaton.getName() << "\n";
    out << "\tDESCRIPTION " << automaton.getDescription() << "\n";
    out << "\tSTART " << automaton.getStartName() << "\n";

    out << "\tFINISH [";
    bool skipComma = true;
    for (const auto& state : automaton.getFinalStates()) {
        if (!skipComma) out << ", ";
        out << state;
        skipComma = false;
    }
    out << "]\n";

    out << "\tVARS\n";
    for (const auto& [name, value] : automaton.getVariables()) {
        out << "\t\t" << name << " = " << value << "\n";
    }
    out << "\tEND\n\n";

    for (const auto& [name, action] : automaton.getStates()) {
        out << "STATE " << name << "\n";
        out << "\tACTION\n";
        out << action; 
        if (action.back() != '\n') out << '\n'; // akce ctene ze zdrojoveho souboru konci \n
        out << "\tEND\n\n";
    }

    for (const auto& t : automaton.getTransitions()) {
        out << "TRANSITION " << t.fromState << " -> " << t.toState << "\n";
        out << "\tCONDITION " << t.condition << "\n";
        out << "\tDELAY " << t.delay << "\n\n";
    }

    out << "END\n";
}
