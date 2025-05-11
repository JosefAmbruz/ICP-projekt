from fsm_core import FSM, State, Transition
import time
import logging

# --- FSM Name: my_fsm ---
# Description: Description

# --- Define FSM Actions and Conditions ---

def action_State_1(variables):
    print("In State 1")


def action_State_2(variables):
    print("In State 2")
    variables.get('x') += 1


def action_State_Final(variables):
    pass


def condition_always_true(variables):
    return True


def condition_xeq2(variables):
    return (variables.get('x')==2)


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
    state_State_Final = State(
        name="State Final",
        action=action_State_Final,
        is_start_state=False,
        is_finish_state=True
    )

    # 3. Define Transitions
    tr_State_1_to_State_Final_0 = Transition(
        target_state_name="State Final",
        condition=condition_xeq2,
        delay=1000.0
    )
    tr_State_2_to_State_1_1 = Transition(
        target_state_name="State 1",
        condition=condition_always_true,
        delay=1000.0
    )
    tr_State_1_to_State_2_2 = Transition(
        target_state_name="State 2",
        condition=condition_always_true,
        delay=1000.0
    )

    # 4. Add Transitions to States
    state_State_1.add_transition(tr_State_1_to_State_Final_0)
    state_State_2.add_transition(tr_State_2_to_State_1_1)
    state_State_1.add_transition(tr_State_1_to_State_2_2)

    # 5. Add States to FSM
    my_fsm.add_state(state_State_1)
    my_fsm.add_state(state_State_2)
    my_fsm.add_state(state_State_Final)

    # 6. Set Initial Variables
    my_fsm.set_variable("x", 0)

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
