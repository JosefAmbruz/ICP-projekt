# FSM Editor and Interpreter

A graphical editor and interpreter for designing, simulating, and running finite state machines (FSMs) with Python integration.

## Table of Contents

- [Features](#features)
- [Requirements](#requirements)
- [Installation](#installation)
- [Building the Project](#building-the-project)
- [Usage](#usage)
- [Project Structure](#project-structure)
- [Doxygen Documentation](#doxygen-documentation)
- [Troubleshooting](#troubleshooting)
- [Contributing](#contributing)
- [License](#license)
- [Authors](#authors)

## Features

- Visual FSM editor with node-based interface
- Add states and transitions
- Python code generation and execution for FSMs
- TCP client-server communication with Python FSM interpreter using custom protocol
- Logging and real-time output display
- Save FSM projects into human readable, custom format

## Incomplete/Missing Functionality (first submission only!)

- Loading FSM from source file
- Editing variables at runtime
- Examples for testing

## Features added in second submission

- ✅ Load a Automaton from **human-readable** and editable text file (`.fsm` file extension)
- ✅ Save current Automaton in the UI editor to a `.fsm` file

  - ➡️ example of a `.fsm` file:

    ```
    #State 2;339;225;1;1
    #State 1;-17;70;1;1
    AUTOMATON my_fsm
    DESCRIPTION "My Automaton"
    START State_Initial
    FINISH [StateA, StateB]
    VARS
    Int x = 0
    String z = hello
    STATE State 1
    ACTION
        x = x + 10
        print(x)
    END
    ...
    ```

- ✅ Show the **current state** of a running Automaton
  - ➡️ Current state of running Autoamton is displayed next to the `🟢Run` button.
- ✅ Added a panel to **display live state** of variables and their values of a running Automaton.
  - ➡️ Use the panel in bottom-right to add variables to your Automaton. Values will be in sync with the running Automaton.
- ✅ Allow for **changing the variable values of a running Automaton**
  - ➡️ While an Automaton is running, you can inject values into pre-defined variables using the bottom-right panel.
- ✅ Improve live logging
  - ➡️ more verbose and straight forward logging
- ✅ Make User Interface more friendly and easy to use.
- ✅ Autamtically detect if the running automaton is stuck.
- ✅ Add examples for testing.

## Requirements

- Qt 5 or Qt 6 (with Widgets and Network modules)
- CMake >= 3.5
- C++17 compatible compiler
- Python 3.x (for FSM interpreter)
- Doxygen (optional, for documentation)

## Installation

1. **Clone the repository:**

   ```sh
   git clone https://github.com/JosefAmbruz/ICP-projekt.git
   cd ICP-projekt
   ```

2. **Install dependencies:**
   - Make sure Qt and Python 3 are installed and available in your PATH.

## Building the Project

1. **Create a build directory:**

   ```sh
   mkdir src/build
   cd src/build
   ```

2. **Configure the project with CMake:**

   ```sh
   cmake ..
   ```

3. **Build:**

   ```sh
   make
   ```

4. **(Optional) Generate Doxygen documentation:**
   ```sh
   doxygen Doxyfile
   ```

## Usage

1. **Run the application:**

   ```sh
   ./icp
   ```

2. **Design your FSM:**

   - Add states and transitions using the graphical editor.
   - Set actions and conditions for states and transitions.
   - Configure variables and initial values.

3. **Generate and run the Python FSM:**

   - Click the **Run** button to generate and execute the Python FSM interpreter.
   - View logs and FSM output in the application.

4. **Connect as a client (optional):**

   - Use the built-in client or your own TCP client to interact with the running FSM.

5. **Save/Load your FSM project:**
   - Use the menu options to save or load FSM designs.

## Project Structure

```
src/                # C++ source code (editor, client, code generation)
src/interpret/      # Python FSM interpreter and core logic
tests/              # Test scripts and client examples
docs/               # (Optional) Doxygen documentation output
```

## Doxygen Documentation

To generate and view documentation:

```sh
doxygen -g           # (first time only, to generate Doxyfile)
doxygen Doxyfile
xdg-open html/index.html
```

## Troubleshooting

- **Python process not starting:**  
  Ensure Python 3 is installed and accessible in your PATH.

- **Client connection refused:**  
  Make sure the Python FSM server is running before connecting the client.

- **Missing Qt modules:**  
  Install the required Qt development packages for your platform.

## Contributing

Contributions are welcome! Please fork the repository and submit a pull request.

## References and Code Attribution

- The implementation of
  `DynamicPortsModel.{cpp, hpp}`, `PortAddRemoveWidget.{cpp, hpp}` is inspired by the _dynamic_ports_ example provided in the repository of the `nodeeditor` library.  
  🔗Link: https://github.com/paceholder/nodeeditor/tree/master/examples

- **_src/nodeeditor-master_** contains the source file of the `nodeeditor` library. The main reason to include source codes of external library this way is that we adjusted the library source code to fit our needs.

## Authors

- Josef Ambruz
- Jakub Kovařík
