#include <iostream>
#include <fstream>
#include <string>
#include "automaton-data.hpp"
#include "parser.hpp"
#include "generator.hpp"

using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " input.txt\n";
        return 1;
    }

    const string inputName = argv[1];
    const string outputName = "output.txt";

    // Parsovani vstupniho souboru se specifikaci

    ifstream inputFile(inputName);
    if (!inputFile) {
        cerr << "Failed to open input file: " << inputName << endl;
        return 1;
    }

    Automaton automaton;
    parseAutomaton(inputFile, automaton);
    inputFile.close();

    // Generovani specifikace do vystupniho souboru
    
    ofstream outputFile(outputName);
    if (!outputFile) {
        cerr << "Failed to create output file: " << outputName << endl;
        return 1;
    }

    generateAutomaton(outputFile, automaton);
    outputFile.close();

    return 0;
}
