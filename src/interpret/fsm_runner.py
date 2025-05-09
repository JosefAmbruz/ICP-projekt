from fsm_core import FSM, State, Transition
import time
import logging # Already configured in fsm_core

# --- Define FSM Actions and Conditions ---
# These would typically be part of the "generated" code or user-defined library

def print_state_entry(variables):
    current_state_name = variables.get('_fsm_current_state_name', 'UnknownState') # FSM will need to inject this
    logging.info(f"Action: Entering state. Counter: {variables.get('counter', 0)}")
    print(f"GUI Action: Entered state {current_state_name}. Counter is {variables.get('counter', 0)}.")

def increment_counter(variables):
    variables['counter'] = variables.get('counter', 0) + 1
    logging.info(f"Action: Counter incremented to {variables['counter']}")
    print(f"GUI Action: Counter incremented to {variables['counter']}.")

def check_counter_less_than_3(variables):
    is_less = variables.get('counter', 0) < 3
    logging.info(f"Condition: counter < 3 ? {is_less}")
    return is_less

def check_counter_greater_equal_3(variables):
    is_ge = variables.get('counter', 0) >= 3
    logging.info(f"Condition: counter >= 3 ? {is_ge}")
    return is_ge

def always_true(variables):
    return True
    
def transition_action_log(variables):
    logging.info("Action: Performing transition specific action.")
    print("GUI Action: Transition action performed.")

# --- Main FSM Execution ---
if __name__ == "__main__":
    # 1. Create the FSM instance
    my_fsm = FSM()

    # 2. Define States
    # For state actions, we can make them aware of the current state if needed,
    # but the FSM class itself already knows the current state.
    # We'll have the FSM inject its current state name into variables for actions to use.
    
    state_A = State(name="State_A", action=print_state_entry, is_start_state=True)
    state_B = State(name="State_B", action=print_state_entry)
    state_C = State(name="State_C", action=print_state_entry, is_finish_state=True)
    state_D_finish = State(name="State_D_Finish", action=print_state_entry, is_finish_state=True)


    # 3. Define Transitions
    # Transition from A to B: increments counter, has a delay
    tr_A_B = Transition(target_state_name="State_B", 
                        condition=always_true, 
                        action=increment_counter, 
                        delay=2.0)
    
    # Transition from B to A: if counter < 3
    tr_B_A = Transition(target_state_name="State_A", 
                        condition=check_counter_less_than_3,
                        action=transition_action_log,
                        delay=1.0)

    # Transition from B to C: if counter >= 3 (finish state)
    tr_B_C = Transition(target_state_name="State_C", 
                        condition=check_counter_greater_equal_3,
                        delay=0.5)

    # Add an alternative finish state accessible from A (e.g., via client command)
    # This transition condition will rely on a variable 'force_finish' set by the client
    def check_force_finish(variables):
        return variables.get('force_finish', False)

    tr_A_D = Transition(target_state_name="State_D_Finish",
                        condition=check_force_finish)

    # 4. Add Transitions to States
    state_A.add_transition(tr_A_B)
    state_A.add_transition(tr_A_D) # New transition
    state_B.add_transition(tr_B_A)
    state_B.add_transition(tr_B_C)

    # 5. Add States to FSM
    my_fsm.add_state(state_A)
    my_fsm.add_state(state_B)
    my_fsm.add_state(state_C)
    my_fsm.add_state(state_D_finish)


    # 6. Set Initial Variables
    my_fsm.set_variable('counter', 0)
    my_fsm.set_variable('force_finish', False) # For the new transition

    # 7. Connect to client and Run the FSM
    # The FSM will wait for a client connection before starting the main loop.
    client_host = 'localhost'
    client_port = 65432 # Use a different port than 12345 if you run client on same machine often
    
    print("Starting FSM...")
    
    # The connect_to_client will block until a client connects or it's stopped.
    # It also starts the client message handler thread.
    my_fsm.connect_to_client(host=client_host, port=client_port)

    if my_fsm._client_socket: # Check if connection was successful
        try:
            # The FSM's action handler can be modified to inject current state name
            # This is a bit of a hack, better would be for actions to receive FSM instance
            # or for FSM to manage this. For now, we will rely on FSM logging.
            # In a real scenario, `print_state_entry` might get the state name from the FSM.
            # Let's assume the FSM sets a special variable `_fsm_current_state_name`
            # (The FSM code doesn't do this automatically, state actions just get 'variables')
            # A better approach: State actions know the state they belong to, or FSM passes state name.
            # For simplicity of the example, the action `print_state_entry` will use a default.
            # The FSM sends CURRENT_STATE message which is the primary way client knows.

            my_fsm.run() # This is a blocking call until FSM finishes or is stopped
        except KeyboardInterrupt:
            print("\nFSM execution interrupted by user (Ctrl+C).")
            my_fsm.stop()
        except Exception as e:
            logging.error(f"An unexpected error occurred during FSM execution: {e}", exc_info=True)
            my_fsm.stop()
        finally:
            # run() method already calls _cleanup(), but an explicit stop here ensures
            # the stop event is set if an exception occurred outside run()'s main loop.
            my_fsm.stop() 
            # Wait for the FSM to fully shutdown if it was running in threads that need joining
            # The _cleanup method in FSM handles joining the client_handler_thread.
            # If run() itself was in a thread (it's not here), you'd join that too.
            print("FSM runner script finished.")
    else:
        print("FSM did not connect to a client. Exiting.")