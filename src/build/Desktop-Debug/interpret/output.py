from fsm_core import FSM, State, Transition
import time
import logging

# --- FSM Name: Automaton ---
# Description: Description

# --- Define FSM Actions and Conditions ---

def action_State_1(variables):
    pass


def action_State_2(variables):
    pass


def condition_always_true(variables):
    return True


# --- Main FSM Execution ---
if __name__ == "__main__":
    # 1. Create the FSM instance
    Automaton = FSM()

    # 2. Define States
    state_State_1 = State(
        name="State 1",
        action=action_State_1,
        is_start_state=True,
        is_finish_state=False
    )
    state_State_2 = State(
        name="State 2",
        action=action_State_2,
        is_start_state=False,
        is_finish_state=False
    )

    # 3. Define Transitions
    tr_State_1_to_State_2_0 = Transition(
        target_state_name="State 2",
        condition=condition_always_true,
        delay=1000.0
    )

    # 4. Add Transitions to States
    state_State_1.add_transition(tr_State_1_to_State_2_0)

    # 5. Add States to FSM
    Automaton.add_state(state_State_1)
    Automaton.add_state(state_State_2)

    # 6. Set Initial Variables
    # No initial variables defined in specification.

    # 7. Connect to client and Run the FSM
    client_host = 'localhost'
    client_port = 65432 # Default port, change if needed

    print(f"Starting FSM 'Automaton'...")
    Automaton.connect_to_client(host=client_host, port=client_port)

    if Automaton._client_socket: # Check if connection was successful
        try:
            Automaton.run()
        except KeyboardInterrupt:
            print("\nFSM execution interrupted by user (Ctrl+C).")
            Automaton.stop()
        except Exception as e:
            logging.error(f"An unexpected error occurred during FSM execution: {e}", exc_info=True)
            Automaton.stop()
        finally:
            Automaton.stop() # Ensure stop is called
            print("FSM runner script finished.")
    else:
        print("FSM did not connect to a client. Exiting.")
