/**
 * @brief Internal Automaton representation
 * @author Jakub Kovarik
 */
#ifndef AUTOMATON_DATA_H
#define AUTOMATON_DATA_H

#include <string>
#include <vector>
#include <unordered_map>

using namespace std;

struct Transition {
    string fromState;
    string toState;
    string condition;  
    int delay = 0;
};

enum class VarDataType
{
    Int,
    Double,
    String
};

struct VariableInfo
{
    string name;
    string value;
    VarDataType type;
};

class Automaton {
    string name;
    string description;
    vector<VariableInfo> variables;
    string startState;
    vector<string> finalStates;
    unordered_map<string, string> states;
    vector<Transition> transitions;

public:
    // Automaton info
    void setName(const string& newName);
    void setDescription(const string& newDescription);
    string getName() const;
    string getDescription() const;

    // Variables
    static string varDataTypeAsString(VarDataType type);
    static VarDataType varDataTypeFromString(const string& str);
    void addVariable(const string& varName, const string& varValue, const VarDataType type);
    vector<VariableInfo> getVariables() const;

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
