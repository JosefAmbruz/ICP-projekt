from fsm_core import FSM, State, Transition
import time
import logging

# --- FSM Name: Automaton ---
# Description: Description

# --- Define FSM Actions and Conditions ---

def _Enter_a_transition_condition(variables):
    pass


def _Enter_code_here(variables):
    pass


def _Enter_code_hereprintHello_there(variables):
    pass


def always_true_condition(variables):
    pass


# --- Main FSM Execution ---
if __name__ == "__main__":
    # 1. Create the FSM instance
    AutomatonFSM()

    # 2. Define States
    state_State_1 = State(
        name="State 1",
        action=iUO_eprintHello_there,
        is_start_state=True,
        is_finish_state=False
    )
    state_State_2 = State(
        name="State 2",
        action=DUeprintHello_there,
        is_start_state=False,
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
    my_fsm.connect_to_client(host=client_host, port=client_port)

    if my_fsm._client_socket: # Check if connection was successful
        try:
            my_fsm.run()
        except KeyboardInterrupt:
            print("\nFSM execution interrupted by user (Ctrl+C).")
            my_fsm.stop()
        except Exception as e:
            logging.error(f"An unexpected error occurred during FSM execution: {e}", exc_info=True)
            my_fsm.stop()
        finally:
            my_fsm.stop() # Ensure stop is called
            print("FSM runner script finished.")
    else:
        print("FSM did not connect to a client. Exiting.")
