from fsm_core import FSM, State, Transition
import time
import logging

# --- FSM Name: MyAutomaton ---
# Description: This is a test automaton.

# --- Define FSM Actions and Conditions ---

def always_true_condition(variables):
    pass


def printIn_State_1(variables):
    pass


def printIn_State_2(variables):
    pass


def printIn_State_3(variables):
    pass


def x_eq_0(variables):
    pass


def x_gt_0(variables):
    pass


def x_lt_0(variables):
    pass


def y_lt_5(variables):
    pass


# --- Main FSM Execution ---
if __name__ == "__main__":
    # 1. Create the FSM instance
    MyAutomaton = FSM()

    # 2. Define States
    state_State3 = State(
        name="State3",
        action=Q1,
        is_start_state=False,
        is_finish_state=True
    )
    state_State2 = State(
        name="State2",
        action=MQY2,
        is_start_state=False,
        is_finish_state=False
    )
    state_State1 = State(
        name="State1",
        action=Y3,
        is_start_state=True,
        is_finish_state=False
    )

    # 3. Define Transitions
    # 4. Add Transitions to States
    # 5. Add States to FSM
    # 6. Set Initial Variables
    # 7. Connect to client and Run the FSM
    client_host = 'localhost'
    client_port = 65432 # Default port, change if needed

    print(f"Starting FSM '{automaton.name}'...")
    MyAutomaton.connect_to_client(host=client_host, port=client_port)

    if MyAutomaton._client_socket: # Check if connection was successful
        try:
            MyAutomaton.run()
        except KeyboardInterrupt:
            print("\nFSM execution interrupted by user (Ctrl+C).")
            MyAutomaton.stop()
        except Exception as e:
            logging.error(f"An unexpected error occurred during FSM execution: {e}", exc_info=True)
            MyAutomaton.stop()
        finally:
            MyAutomaton.stop() # Ensure stop is called
            print("FSM runner script finished.")
    else:
        print("FSM did not connect to a client. Exiting.")
