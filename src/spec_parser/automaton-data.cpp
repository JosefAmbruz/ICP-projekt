/**
 * @brief A class representning an automaton
 * @author Jakub Kovarik
 */

#include "automaton-data.hpp"
#include <algorithm>

// Automaton info
void Automaton::setName(const string& newName) {name = newName;}
void Automaton::setDescription(const string& newDescription) {description = newDescription;}
string Automaton::getName() const {return name;}
string Automaton::getDescription() const {return description;}

// Variable
void Automaton::addVariable(const string& varName, const string& varValue) {
    variables[varName] = varValue;
}

void Automaton::setVariableValue(const string& varName, const string& newValue) {
    if (variables.find(varName) != variables.end()) {
        variables[varName] = newValue;
    }
}

bool Automaton::hasVariable(const string& varName) const {
    return variables.find(varName) != variables.end();
}

string Automaton::getVariableValue(const string& varName) const {
    auto var = variables.find(varName);
    return (var != variables.end()) ? var->second : "";
}

unordered_map<string, string> Automaton::getVariables() const {return variables;}

// State
void Automaton::addState(const string& stateName, const string& action) { 
    states[stateName] = action;
}

void Automaton::appendToAction(const std::string& stateName, const std::string& line) {
    states[stateName] += line + "\n";
}

void Automaton::setStartState(const string& stateName) {
    startState = stateName;
}

void Automaton::addFinalState(const string& stateName) { 
    if (!isFinalState(stateName))
        finalStates.push_back(stateName);
}

bool Automaton::isFinalState(const string& stateName) const {
    return find(finalStates.begin(), finalStates.end(), stateName) != finalStates.end();
}

string Automaton::getStateAction(const string& stateName) const {
    auto state = states.find(stateName);
    return (state != states.end()) ? state->second : "";
}

unordered_map<string, string> Automaton::getStates() const {return states;}

vector<string> Automaton::getFinalStates() const {return finalStates;}

string Automaton::getStartName() const {return startState;}

// Transition
void Automaton::addTransition(const Transition& t) {
    transitions.push_back(t);
}

vector<Transition> Automaton::getTransitions() const {
    return transitions;
}

vector<Transition> Automaton::getTransitionsFrom(const string& stateName) const {
    vector<Transition> result;
    for (const auto& t : transitions) {
        if (t.fromState == stateName) {
            result.push_back(t);
        }
    }
    return result;
}
