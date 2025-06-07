
/**
 * @file interpret_generator.h
 * @brief Header file for the InterpretGenerator class and related utility functions.
 * 
 * This file contains the declaration of the InterpretGenerator class, which is responsible
 * for generating Python FSM (Finite State Machine) scripts from an Automaton object. It also
 * includes utility functions for sanitizing strings, escaping Python literals, and transforming
 * code to work with Python dictionaries.
 * 
 * The generated Python scripts include state and transition definitions, action and condition
 * functions, and code to run the FSM and connect to a client.
 * 
 * The utility functions provided in this file help ensure that the generated Python code is
 * valid, safe, and adheres to Python syntax rules.
 * 
 * Dependencies:
 * - Qt framework (QObject, QFile, QTextStream)
 * - Standard C++ libraries (algorithm, set)
 * - Automaton data structure (spec_parser/automaton-data.hpp)
 * 
 * @author Josef Ambruz
 * @date 2025-5-11
 */

#ifndef INTERPRET_GENERATOR_H
#define INTERPRET_GENERATOR_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <algorithm> // For std::replace, std::remove_if
#include <set>       // For ordered unique function names

#include "spec_parser/automaton-data.hpp"

/**
 * @brief Sanitizes a string to be a valid Python identifier.
 * 
 * Replaces spaces and invalid characters with underscores, handles Python keywords,
 * and ensures the identifier does not start with a digit.
 * 
 * @param name The input string to sanitize.
 * @return QString The sanitized Python identifier.
 */
QString sanitize_python_identifier(std::string name);

/**
 * @brief Escapes a string to be a valid Python string literal.
 * 
 * @param s The input string.
 * @return QString The escaped Python string literal.
 */
QString to_python_string_literal(const std::string& s);

/**
 * @brief Converts a string value to a Python literal (bool, int, float, or string).
 * 
 * @param val_str The input value as string.
 * @return QString The Python literal representation.
 */
QString to_python_value_literal(const std::string& val_str);

/**
 * @brief Generates Python code that creates local variables from the variables dictionary,
 * inserts the user action code, and writes back the local variables to the dictionary.
 * 
 * @param code The user action code.
 * @param variables The map of variable names and their default values.
 * @return QString The transformed Python code.
 */
QString transform_to_local_vars(const QString& code, const std::vector<VariableInfo>& variables);

/**
 * @brief Replaces variable names in code with variables.get('<name>').
 * 
 * @param code The code in which to replace variable names.
 * @param variables The map of variable names.
 * @return QString The code with variables replaced by variables.get().
 */
QString replace_variables_with_get(const QString& code, const std::vector<VariableInfo>& variables);

class InterpretGenerator : public QObject
{
    Q_OBJECT
public:

    /**
     * @brief Constructor for InterpretGenerator.
     * 
     * @param parent The parent QObject.
     */
    explicit InterpretGenerator(QObject *parent = nullptr);

    /**
     * @brief Generates a Python FSM script from the given Automaton and writes it to the specified file.
     * 
     * The generated script includes state and transition definitions, action and condition functions,
     * and code to run the FSM and connect to a client.
     * 
     * @param automaton The Automaton to generate the Python script from.
     * @param output_filename The path to the output Python file.
     */
    void generate(const Automaton& automaton, const QString& output_filename);

signals:
};

#endif // INTERPRET_GENERATOR_H
