/**
 * @brief Ziskani dat z textove specifikace automatu
 * @todo Kontrola redefinice promennych/stavu/prechodu, 
 *       zda boli definovane start a finish stavy,
 *       pridat inputEvent u prechodu?
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include "automaton-data.hpp" // trida Automaton

enum class ParserState {
    EXPECT_AUTOMATON,
    EXPECT_DESCRIPTION,
    EXPECT_START,
    EXPECT_FINISH,
    EXPECT_VARS,
    INSIDE_VARS,
    EXPECT_STATE_OR_TRANSITION,
    EXPECT_STATE_ACTION,
    INSIDE_STATE_ACTION,
    EXPECT_TRANSITION_CONDITION,
    EXPECT_TRANSITION_DELAY,
    DONE
};

// Kontrola klicoveho slova na zacatku radku
bool startsWith(const string& str, const string& prefix) {
    return str.size() >= prefix.size() &&
           str.compare(0, prefix.size(), prefix) == 0;
}

// Orezani komentaru a prazdnych znaku na zacatku/konci retezce
string trim(const string& str) {
    size_t hash = str.find('#');
    string noComment = (hash != string::npos) ? str.substr(0, hash) : str;

    size_t first = noComment.find_first_not_of(" \t\r\n");
    if (first == string::npos) return "";

    size_t last = noComment.find_last_not_of(" \t\r\n");
    return noComment.substr(first, (last - first + 1));
}

void parseAutomaton(istream& in, Automaton& automaton) {
    string rawLine;
    string line;
    ParserState state = ParserState::EXPECT_AUTOMATON;
    string currentState;
    Transition currentTransition;

    while (getline(in, rawLine)) {
        line = trim(rawLine);
        if (line.empty()) continue;

        switch (state) {
            case ParserState::EXPECT_AUTOMATON:
                if (startsWith(line, "AUTOMATON ")) {
                    automaton.setName(trim(line.substr(10)));
                    state = ParserState::EXPECT_DESCRIPTION;
                } else {
                    cerr << "Expected 'AUTOMATON', found: " << line << endl;
                }
                break;

            case ParserState::EXPECT_DESCRIPTION:
                if (startsWith(line, "DESCRIPTION ")) {
                    automaton.setDescription(trim(line.substr(12)));
                    state = ParserState::EXPECT_START;
                } else {
                    cerr << "Expected 'DESCRIPTION', found: " << line << endl;
                }
                break;

            case ParserState::EXPECT_START:
                if (startsWith(line, "START ")) {
                    automaton.setStartState(trim(line.substr(6)));
                    state = ParserState::EXPECT_FINISH;
                } else {
                    cerr << "Expected 'START', found: " << line << endl;
                }
                break;

            case ParserState::EXPECT_FINISH:
                if (startsWith(line, "FINISH ")) {
                    string rest = trim(line.substr(7));
                    rest.erase(remove(rest.begin(), rest.end(), '['), rest.end());
                    rest.erase(remove(rest.begin(), rest.end(), ']'), rest.end());

                    istringstream ss(rest);
                    string stateName;
                    while (getline(ss, stateName, ',')) {
                        automaton.addFinalState(trim(stateName));
                    }
                    state = ParserState::EXPECT_VARS;
                } else {
                    cerr << "Expected 'FINISH', found: " << line << endl;
                }
                break;

            case ParserState::EXPECT_VARS:
                if (line == "VARS") {
                    state = ParserState::INSIDE_VARS;
                } else {
                    cerr << "Expected 'VARS', found: " << line << endl;
                }
                break;

            case ParserState::INSIDE_VARS: {
                if (line == "END") {
                    state = ParserState::EXPECT_STATE_OR_TRANSITION;
                    break;
                }
                size_t eq = line.find('=');
                if (eq != string::npos) {
                    string name = trim(line.substr(0, eq));
                    string value = trim(line.substr(eq + 1));
                    automaton.addVariable(name, value);
                } else {
                    cerr << "Malformed VARS line: " << line << endl;
                }
                break;
            }

            case ParserState::EXPECT_STATE_OR_TRANSITION:
                if (startsWith(line, "STATE ")) {
                    currentState = trim(line.substr(6));
                    automaton.addState(currentState);
                    state = ParserState::EXPECT_STATE_ACTION;
                } else if (startsWith(line, "TRANSITION ")) {
                    size_t arrow = line.find("->");
                    if (arrow != string::npos) {
                        currentTransition.fromState = trim(line.substr(10, arrow - 10));
                        currentTransition.toState = trim(line.substr(arrow + 2));
                        state = ParserState::EXPECT_TRANSITION_CONDITION;
                    } else {
                        cerr << "Malformed TRANSITION line: " << line << endl;
                    }
                } else if (line == "END") {
                    // Posledny end
                    state = ParserState::DONE;
                } else {
                    cerr << "Expected 'STATE', 'TRANSITION', or 'END', found: " << line << endl;
                }
                break;

            case ParserState::EXPECT_STATE_ACTION:
                if (line == "ACTION") {
                    state = ParserState::INSIDE_STATE_ACTION;
                } else {
                    cerr << "Expected 'ACTION', found: " << line << endl;
                }
                break;

            case ParserState::INSIDE_STATE_ACTION:
                if (line == "END") {
                    state = ParserState::EXPECT_STATE_OR_TRANSITION;
                } else {
                    automaton.appendToAction(currentState, rawLine); // zachovalo sa odsadenie?
                }
                break;
            
            case ParserState::EXPECT_TRANSITION_CONDITION:
                if (startsWith(line, "CONDITION ")) {
                    currentTransition.condition = trim(line.substr(10));
                    state = ParserState::EXPECT_TRANSITION_DELAY;
                } else {
                    cerr << "Expected 'CONDITION', found: " << line << endl;
                }
                break;

            case ParserState::EXPECT_TRANSITION_DELAY:
                if (startsWith(line, "DELAY ")) {
                    string delayStr = trim(line.substr(6));
                    size_t pos = 0;
                    try {
                        int delay = stoi(delayStr, &pos);
                        if (pos != delayStr.size()) {
                            cerr << "Invalid characters in DELAY" << delayStr << endl;
                        } else {
                            currentTransition.delay = delay;
                        }
                    } catch (const invalid_argument& e) {
                        cerr << "Invalid delay value: " << delayStr << endl;
                    } catch (const out_of_range& e) {
                        cerr << "DELAY value out of range: " << delayStr << endl;
                    }
                    automaton.addTransition(currentTransition);
                    state = ParserState::EXPECT_STATE_OR_TRANSITION;
                } else {
                    cerr << "Expected 'DELAY', found: " << line << endl;
                }
                break;

            case ParserState::DONE:
                break;
        }
    }
}


// TEST
int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " input.txt\n";
        return 1;
    }

    ifstream file(argv[1]);
    if (!file) {
        std::cerr << "Failed to open file: " << argv[1] << "\n";
        return 1;
    }

    Automaton automaton;
    parseAutomaton(file, automaton);

    cout << "Automaton name: " << automaton.getName() << "\n";
    cout << "Description: " << automaton.getDescription() << "\n";
    cout << "Start state: " << automaton.getStartName() << "\n";

    cout << "Final states:\n";
    for (const auto& s : automaton.getFinalStates()) {
        cout << "  " << s << "\n";
    }

    cout << "Variables:\n";
    for (const auto& [name, value] : automaton.getVariables()) {
        cout << "  " << name << " = " << value << "\n";
    }

    cout << "States:\n";
    for (const auto& [name, _] : automaton.getStates()) {
        cout << "  " << name << ":\n";
        cout << automaton.getStateAction(name) << "\n";
    }

    cout << "Transitions:\n";
    for (const auto& tr : automaton.getTransitions()) {
        cout << "  " << tr.fromState << " -> " << tr.toState << "\n";
        cout << "    Condition: " << tr.condition << "\n";
        cout << "    Delay: " << tr.delay << "\n";
    }

    return 0;
}
