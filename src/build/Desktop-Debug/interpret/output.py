from fsm_core import FSM, State, Transition
import time
import logging

# --- FSM Name: my_fsm ---
# Description: Description

# --- Define FSM Actions and Conditions ---

def action_State_1(fsm, variables):
    pass


def action_State_2(fsm, variables):
    pass


def condition_always_true(fsm, variables):
    return True


# --- Main FSM Execution ---
if __name__ == "__main__":
    # 1. Create the FSM instance
    my_fsm = FSM()

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
        delay=10.0
    )

    # 4. Add Transitions to States
    state_State_1.add_transition(tr_State_1_to_State_2_0)

    # 5. Add States to FSM
    my_fsm.add_state(state_State_1)
    my_fsm.add_state(state_State_2)

    # 6. Set Initial Variables
    # No initial variables defined in specification.

    # 7. Connect to client and Run the FSM
    client_host = 'localhost'
    client_port = 65432 # Default port, change if needed

    print(f"Starting FSM 'my_fsm'...")
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
