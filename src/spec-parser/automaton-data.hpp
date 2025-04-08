#ifndef AUTOMATON_DATA_H
#define AUTOMATON_DATA_H

#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>

using namespace std;

struct Transition {
    string fromState;
    string toState;
    string condition;  // Bool vyraz nad promemennymi
    string inputEvent; // Pozadovana vstupni udalost TODO!!!
    int delay = 0;
};

class Automaton {
    string name;
    string description;
    unordered_map<string, string> variables;    // <name, value>
    string startState;
    vector<string> finalStates;
    unordered_map<string, string> states;       // <name, action>
    vector<Transition> transitions;

public:
    // Automaton info
    void setName(const string& newName);
    void setDescription(const string& newDescription);
    string getName() const;
    string getDescription() const;

    // Variables
    void addVariable(const string& varName, const string& varValue = "");
    void setVariableValue(const string& varName, const string& newValue);
    bool hasVariable(const string& varName) const;
    string getVariableValue(const string& varName) const;
    unordered_map<string, string> getVariables() const;

    // States
    void addState(const string& stateName, const string& action = "");
    void appendToAction(const std::string& stateName, const std::string& line);
    void setStartState(const string& stateName);
    void addFinalState(const string& stateName);
    bool isFinalState(const string& stateName) const;
    string getStateAction(const string& stateName) const;
    unordered_map<string, string> getStates() const;
    vector<string> getFinalStates() const;
    string getStartName() const;
    
    // Transitions
    void addTransition(const Transition& t);
    vector<Transition> getTransitions() const;
    vector<Transition> getTransitionsFrom(const string& stateName) const;
};

#endif // AUTOMATON_DATA_H