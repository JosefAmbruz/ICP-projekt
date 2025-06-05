from fsm_core import FSM, State, Transition
import time
import logging

# --- FSM Name: my_fsmm  ---
# Description: Description

# --- Define FSM Actions and Conditions ---

def action_State_1(fsm, variables):
    x = variables.get('x')
    
    print(x)
    fsm.set_variable('x', x)
    


def action_State_2(fsm, variables):
    pass


def condition_always_true(fsm, variables):
    return True


# --- Main FSM Execution ---
if __name__ == "__main__":
    # 1. Create the FSM instance
    my_fsmm_ = FSM()

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
        is_finish_state=True
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
    my_fsmm_.add_state(state_State_1)
    my_fsmm_.add_state(state_State_2)

    # 6. Set Initial Variables
    my_fsmm_.set_variable("x", 0)

    # 7. Connect to client and Run the FSM
    client_host = 'localhost'
    client_port = 65432 # Default port, change if needed

    print(f"Starting FSM 'my_fsmm_'...")
    my_fsmm_.connect_to_client(host=client_host, port=client_port)

    if my_fsmm_._client_socket: # Check if connection was successful
        try:
            my_fsmm_.run()
        except KeyboardInterrupt:
            print("\nFSM execution interrupted by user (Ctrl+C).")
            my_fsmm_.stop()
        except Exception as e:
            logging.error(f"An unexpected error occurred during FSM execution: {e}", exc_info=True)
            my_fsmm_.stop()
        finally:
            my_fsmm_.stop() # Ensure stop is called
            print("FSM runner script finished.")
    else:
        print("FSM did not connect to a client. Exiting.")
