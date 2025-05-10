from fsm_core import FSM, State, Transition
import time
import logging

# --- FSM Name: MyAutomaton ---
# Description: This is a test automaton.

# --- Define FSM Actions and Conditions ---

def action_State1(variables):
    print('In State 1')
    print('testing new line')


def action_State2(variables):
    print('In State 2')


def action_State3(variables):
    print('In State 3')


def condition_always_true(variables):
    return True


def condition_x_eq_0(variables):
    x == 0


def condition_x_gt_0(variables):
    x > 0


def condition_x_lt_0(variables):
    x < 0


def condition_y_lt_5(variables):
    y < 5


# --- Main FSM Execution ---
if __name__ == "__main__":
    # 1. Create the FSM instance
    MyAutomaton = FSM()

    # 2. Define States
    state_State3 = State(
        name="State3",
        action=action_State3,
        is_start_state=False,
        is_finish_state=True
    )
    state_State2 = State(
        name="State2",
        action=action_State2,
        is_start_state=False,
        is_finish_state=False
    )
    state_State1 = State(
        name="State1",
        action=action_State1,
        is_start_state=True,
        is_finish_state=False
    )

    # 3. Define Transitions
    tr_State1_to_State2_0 = Transition(
        target_state_name="State2",
        condition=condition_x_gt_0,
        delay=0.0
    )
    tr_State2_to_State3_1 = Transition(
        target_state_name="State3",
        condition=condition_y_lt_5,
        delay=0.0
    )
    tr_State3_to_State1_2 = Transition(
        target_state_name="State1",
        condition=condition_x_eq_0,
        delay=0.0
    )
    tr_State1_to_State3_3 = Transition(
        target_state_name="State3",
        condition=condition_x_lt_0,
        delay=0.0
    )

    # 4. Add Transitions to States
    state_State1.add_transition(tr_State1_to_State2_0)
    state_State2.add_transition(tr_State2_to_State3_1)
    state_State3.add_transition(tr_State3_to_State1_2)
    state_State1.add_transition(tr_State1_to_State3_3)

    # 5. Add States to FSM
    MyAutomaton.add_state(state_State3)
    MyAutomaton.add_state(state_State2)
    MyAutomaton.add_state(state_State1)

    # 6. Set Initial Variables
    MyAutomaton.set_variable("y", 0)
    MyAutomaton.set_variable("x", 0)

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
