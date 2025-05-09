#include "interpret_generator.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>

// Helper to make a string safe as a Python identifier
QString sanitize_python_identifier(std::string name) {
    if (name.empty()) {
        return "_empty_name_placeholder_"; // Should not happen if names are required
    }

    // Replace spaces and invalid characters with underscores
    std::replace(name.begin(), name.end(), ' ', '_');
    std::replace(name.begin(), name.end(), '-', '_');
    std::replace(name.begin(), name.end(), '.', '_');

    // Remove any characters not suitable for identifiers
    name.erase(std::remove_if(name.begin(), name.end(), [](char c) {
                   return !isalnum(c) && c != '_';
               }), name.end());

    // Ensure it doesn't start with a digit (prepend underscore if so)
    if (isdigit(name[0])) {
        name = "_" + name;
    }

    // Basic check for Python keywords
    static const std::set<std::string> keywords = {"None", "True", "False", "def", "class", "if", "else", "elif", "for", "while", "return", "import", "from"};
    if (keywords.count(name)) {
        name += "_var";
    }

    return QString::fromStdString(name);
}

// Helper to create a Python string literal (e.g., "value" -> "\"value\"")
QString to_python_string_literal(const std::string& s) {
    std::string escaped_s = s;
    // Basic escaping: replace \ with \\ and " with \"
    size_t pos = 0;

    while ((pos = escaped_s.find('\\', pos)) != std::string::npos) {
        escaped_s.replace(pos, 1, "\\\\");
        pos += 2;
    }

    pos = 0;

    while ((pos = escaped_s.find('\"', pos)) != std::string::npos) {
        escaped_s.replace(pos, 1, "\\\"");
        pos += 2;
    }

    return "\"" + QString::fromStdString(escaped_s) + "\"";
}

// Helper to convert string value to Python literal (bool, int, float, or string)
QString to_python_value_literal(const std::string& val_str) {
    if (val_str.empty()) return "None"; // Maybe error instead?

    std::string lower_val = val_str;
    std::transform(lower_val.begin(), lower_val.end(), lower_val.begin(), ::tolower);

    if (lower_val == "true") return "True";
    if (lower_val == "false") return "False";

    // Try to parse as int
    try {
        size_t processed_chars;
        int i_val = std::stoi(val_str, &processed_chars);
        if (processed_chars == val_str.length()) { // Ensure entire string was parsed
            return QString::fromStdString(std::to_string(i_val));
        }
    } catch (const std::invalid_argument&) {
    } catch (const std::out_of_range&) {}

    // Try to parse as float
    try {
        size_t processed_chars;
        double d_val = std::stod(val_str, &processed_chars);
        if (processed_chars == val_str.length()) { // Ensure entire string was parsed
            std::ostringstream oss;
            oss << d_val; // Let C++ decide precision, or use std::fixed, std::setprecision
            return QString::fromStdString(oss.str());
        }
    } catch (const std::invalid_argument&) {
    } catch (const std::out_of_range&) {}

    // Default to string literal
    return to_python_string_literal(val_str);
}


InterpretGenerator::InterpretGenerator(QObject *parent)
    : QObject{parent}
{

}

void InterpretGenerator::generate(const Automaton& automaton, const QString& output_filename) {
    QDir().mkpath(QFileInfo(output_filename).absolutePath()); // Ensure directory exists
    
    QFile file(output_filename);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to open file for writing:" << output_filename;
        return;
    }

    QTextStream outfile(&file);

    // --- Collect all function names ---
    std::set<QString> function_names; // function names are sorted and unique
    function_names.insert("always_true_condition"); // default condition

    for (const auto& pair : automaton.getStates()) {
        if (!pair.second.empty()) { // action
            function_names.insert(sanitize_python_identifier(pair.second));
        }
    }

    for (const auto& transition : automaton.getTransitions()) {
        if (!transition.condition.empty()) {
            function_names.insert(sanitize_python_identifier(transition.condition));
        }
    }

    // --- Python code generation ---
    outfile << "from fsm_core import FSM, State, Transition\n";
    outfile << "import time\n";
    outfile << "import logging\n\n";

    outfile << "# --- FSM Name: " << QString::fromStdString(automaton.getName()) << " ---\n";
    if (!automaton.getDescription().empty()) {
        outfile << "# Description: " << QString::fromStdString(automaton.getDescription()) << "\n";
    }
    outfile << "\n";

    outfile << "# --- Define FSM Actions and Conditions ---\n\n";

    for (const auto& func_name : function_names) {
        outfile << "def " << func_name << "(variables):\n";
        outfile << "    pass\n";
        outfile << "\n\n";
    }

    outfile << "# --- Main FSM Execution ---\n";
    outfile << "if __name__ == \"__main__\":\n";
    outfile << "    # 1. Create the FSM instance\n";

    QString fsm_name = sanitize_python_identifier(automaton.getName());
    outfile << "    " << fsm_name << "FSM()\n\n";

    outfile << "    # 2. Define States\n";
    std::unordered_map<string, string> states = automaton.getStates();

    for (const auto& state : states) {
        QString state_name = sanitize_python_identifier(state.first);
        outfile << "    state_" << state_name << " = State(\n";
        outfile << "        name=" << to_python_string_literal(state.first) << ",\n";

        QString action_func = "None";
        auto it_state_action = automaton.getStates().find(state.first);
        if (it_state_action != automaton.getStates().end() && !it_state_action->second.empty()) {
            action_func = sanitize_python_identifier(it_state_action->second);
        }
        outfile << "        action=" << action_func << ",\n";
        outfile << "        is_start_state=" << (state.first == automaton.getStartName() ? "True" : "False") << ",\n";
        outfile << "        is_finish_state=" << (automaton.isFinalState(state.first) ? "True" : "False") << "\n";
        outfile << "    )\n";
    }

    outfile << "\n";

    outfile << "    # 3. Define Transitions\n";


    outfile << "    # 4. Add Transitions to States\n";


    outfile << "    # 5. Add States to FSM\n";


    outfile << "    # 6. Set Initial Variables\n";

    outfile << "    # 7. Connect to client and Run the FSM\n";
    outfile << "    client_host = 'localhost'\n";
    outfile << "    client_port = 65432 # Default port, change if needed\n\n";
    outfile << "    print(f\"Starting FSM '{automaton.name}'...\")\n";
    outfile << "    my_fsm.connect_to_client(host=client_host, port=client_port)\n\n";
    outfile << "    if my_fsm._client_socket: # Check if connection was successful\n";
    outfile << "        try:\n";
    outfile << "            my_fsm.run()\n";
    outfile << "        except KeyboardInterrupt:\n";
    outfile << "            print(\"\\nFSM execution interrupted by user (Ctrl+C).\")\n";
    outfile << "            my_fsm.stop()\n";
    outfile << "        except Exception as e:\n";
    outfile << "            logging.error(f\"An unexpected error occurred during FSM execution: {e}\", exc_info=True)\n";
    outfile << "            my_fsm.stop()\n";
    outfile << "        finally:\n";
    outfile << "            my_fsm.stop() # Ensure stop is called\n";
    outfile << "            print(\"FSM runner script finished.\")\n";
    outfile << "    else:\n";
    outfile << "        print(\"FSM did not connect to a client. Exiting.\")\n";




    file.close();
}
