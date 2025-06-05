from fsm_core import FSM, State, Transition
import time
import logging

# --- FSM Name: my_fsm ---
# Description: Description

# --- Define FSM Actions and Conditions ---

def action_Final_Wrong(fsm, variables):
    pass


def action_Finnal_Correct(fsm, variables):
    pass


def action_State_1(fsm, variables):
    pass


def action_Waiting(fsm, variables):
    pass


def condition_always_true(fsm, variables):
    return True


def condition_x_eq_1(fsm, variables):
    return (variables.get('x') == 1)


def condition_x_ne_1(fsm, variables):
    return (variables.get('x') != 1)


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
    state_Waiting = State(
        name="Waiting",
        action=action_Waiting,
        is_start_state=False,
        is_finish_state=False
    )
    state_Finnal_Correct = State(
        name="Finnal Correct",
        action=action_Finnal_Correct,
        is_start_state=False,
        is_finish_state=True
    )
    state_Final_Wrong = State(
        name="Final Wrong",
        action=action_Final_Wrong,
        is_start_state=False,
        is_finish_state=True
    )

    # 3. Define Transitions
    tr_Waiting_to_Final_Wrong_0 = Transition(
        target_state_name="Final Wrong",
        condition=condition_x_eq_1,
        delay=10000.0
    )
    tr_Waiting_to_Finnal_Correct_1 = Transition(
        target_state_name="Finnal Correct",
        condition=condition_x_ne_1,
        delay=3000.0
    )
    tr_State_1_to_Waiting_2 = Transition(
        target_state_name="Waiting",
        condition=condition_always_true,
        delay=5000.0
    )

    # 4. Add Transitions to States
    state_Waiting.add_transition(tr_Waiting_to_Final_Wrong_0)
    state_Waiting.add_transition(tr_Waiting_to_Finnal_Correct_1)
    state_State_1.add_transition(tr_State_1_to_Waiting_2)

    # 5. Add States to FSM
    my_fsm.add_state(state_State_1)
    my_fsm.add_state(state_Waiting)
    my_fsm.add_state(state_Finnal_Correct)
    my_fsm.add_state(state_Final_Wrong)

    # 6. Set Initial Variables
    my_fsm.set_variable("x", 1)

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
