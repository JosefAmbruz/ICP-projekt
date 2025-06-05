import time
import socket
import json
import threading
import logging

# Configure basic logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - FSM - %(message)s')

class Transition:
    def __init__(self, target_state_name, condition=None, action=None, delay=0.0):
        """
        Represents a transition between states.

        Args:
            target_state_name (str): The name of the state this transition leads to.
            condition (callable, optional): A function that takes an FSM instance and a
                                           dictionary of FSM variables, returning True if
                                           the transition can be taken. Defaults to always True.
            action (callable, optional): A function to execute when this transition is taken.
                                        It takes a dictionary of FSM variables (live).
            delay (float, optional): Time in miliseconds to wait before completing the transition.
                                     Defaults to 0.0.
        """
        self.target_state_name = target_state_name
        # Condition now expects (fsm_instance, variables_dict)
        self.condition = condition if callable(condition) else lambda _fsm, _variables: True
        self.action = action
        self.delay = delay # Assumed to be in seconds

    def __repr__(self):
        return f"<Transition to '{self.target_state_name}' delay={self.delay}s>"

class State:
    def __init__(self, name, action=None, is_start_state=False, is_finish_state=False):
        """
        Represents a state in the FSM.

        Args:
            name (str): The unique name of the state.
            action (callable, optional): A function to execute upon entering this state.
                                        It takes (fsm_instance, variables_dict_copy).
            is_start_state (bool, optional): True if this is the starting state. Defaults to False.
            is_finish_state (bool, optional): True if this is a finish state. Defaults to False.
        """
        self.name = name
        self.action = action
        self.is_start_state = is_start_state
        self.is_finish_state = is_finish_state
        self.transitions = []

    def add_transition(self, transition):
        """Adds a transition originating from this state."""
        if not isinstance(transition, Transition):
            raise TypeError("transition must be an instance of Transition class")
        self.transitions.append(transition)

    def __repr__(self):
        return f"<State '{self.name}' Start={self.is_start_state} Finish={self.is_finish_state}>"

class FSM:
    def __init__(self):
        self.states = {}  # name: State object
        self.variables = {}
        self.current_state = None
        self.start_state_name = None
        self._client_socket = None
        self._client_address = None
        self._stop_event = threading.Event()
        self._variable_lock = threading.Lock() # To protect access to self.variables
        self._client_handler_thread = None

        # For interruptible delays
        self._re_evaluate_event = threading.Event()
        self._current_delay_target_transition = None # Stores the Transition object being delayed
        self._current_delay_end_time = None          # Stores the end time for the current delay

    def add_state(self, state):
        if not isinstance(state, State):
            raise TypeError("state must be an instance of State class")
        if state.name in self.states:
            raise ValueError(f"State with name '{state.name}' already exists.")
        self.states[state.name] = state
        if state.is_start_state:
            if self.start_state_name is not None:
                raise ValueError("Multiple start states defined. Only one is allowed.")
            self.start_state_name = state.name
        logging.info(f"Added state: {state.name}")

    def set_variable(self, name, value):
        with self._variable_lock:
            self.variables[name] = value
        logging.info(f"Variable '{name}' set to '{value}'")
        self._send_to_client("VARIABLE_UPDATE", {"name": name, "value": value})

        # If FSM is in a delay, signal re-evaluation
        # Check stop_event to avoid signaling if FSM is already stopping
        if self._current_delay_target_transition and not self._stop_event.is_set():
            logging.debug(f"Signaling re-evaluation for state {self.current_state.name} due to variable change during delay.")
            self._re_evaluate_event.set()


    def get_variable(self, name, default=None):
        with self._variable_lock:
            return self.variables.get(name, default)

    def _send_to_client(self, message_type, payload=None):
        if self._client_socket:
            try:
                message = {"type": message_type, "payload": payload or {}}
                self._client_socket.sendall((json.dumps(message) + "\n").encode('utf-8'))
            except (socket.error, BrokenPipeError) as e:
                logging.error(f"Error sending message to client: {e}. Client might have disconnected.")
                self._handle_disconnection()


    def _handle_client_messages(self):
        buffer = ""
        try:
            while not self._stop_event.is_set() and self._client_socket:
                try:
                    self._client_socket.settimeout(0.5)
                    data = self._client_socket.recv(1024)
                    if not data:
                        logging.info("Client disconnected gracefully.")
                        self._handle_disconnection()
                        break
                    
                    buffer += data.decode('utf-8')
                    
                    while '\n' in buffer:
                        message_str, buffer = buffer.split('\n', 1)
                        if not message_str.strip(): continue
                        try:
                            message = json.loads(message_str)
                            logging.info(f"Received from client: {message}")
                            if message.get("type") == "SET_VARIABLE":
                                var_name = message.get("payload", {}).get("name")
                                var_value = message.get("payload", {}).get("value")
                                if var_name is not None:
                                    self.set_variable(var_name, var_value)
                            elif message.get("type") == "STOP_FSM":
                                logging.info("Received STOP_FSM command from client.")
                                self.stop()
                        except json.JSONDecodeError:
                            logging.warning(f"Invalid JSON received from client: {message_str}")
                        except Exception as e:
                            logging.error(f"Error processing client message: {e}")
                except socket.timeout:
                    continue 
                except socket.error as e:
                    logging.error(f"Socket error in client handler: {e}")
                    self._handle_disconnection()
                    break
        except Exception as e:
            logging.error(f"Client handler thread encountered an error: {e}")
        finally:
            logging.info("Client message handler thread finished.")


    def _handle_disconnection(self):
        if self._client_socket:
            logging.info(f"Handling disconnection from {self._client_address}")
            # self._send_to_client("FSM_ERROR", {"message": "Client disconnected or connection lost."}) # Might fail if socket is bad
            self.stop() 
            try:
                self._client_socket.close()
            except socket.error:
                pass 
            self._client_socket = None


    def connect_to_client(self, host='localhost', port=12345):
        server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        try:
            server_socket.bind((host, port))
            server_socket.listen(1)
            logging.info(f"FSM Server listening on {host}:{port}")
            print(f"FSM Server: Waiting for a client connection on {host}:{port}...")
            
            server_socket.settimeout(1.0) 
            while not self._stop_event.is_set():
                try:
                    conn, addr = server_socket.accept()
                    self._client_socket = conn
                    self._client_address = addr
                    logging.info(f"Client connected from {addr}")
                    print(f"FSM Server: Client connected from {addr}")
                    self._send_to_client("FSM_CONNECTED", {"message": "Successfully connected to FSM."})
                    
                    self._client_handler_thread = threading.Thread(target=self._handle_client_messages, daemon=True)
                    self._client_handler_thread.start()
                    break 
                except socket.timeout:
                    continue 
                except Exception as e:
                    logging.error(f"Error accepting connection: {e}")
                    self.stop() 
                    break
        except Exception as e:
            logging.error(f"Could not start FSM server: {e}")
            self.stop() 
        finally:
            server_socket.close() 

        if not self._client_socket and not self._stop_event.is_set():
            logging.error("Failed to connect to any client. FSM cannot run.")
            self.stop()


    def run(self):
        if not self.start_state_name:
            logging.error("No start state defined for the FSM.")
            self._send_to_client("FSM_ERROR", {"message": "No start state defined."})
            return

        if self.start_state_name not in self.states:
            logging.error(f"Start state '{self.start_state_name}' not found in defined states.")
            self._send_to_client("FSM_ERROR", {"message": f"Start state '{self.start_state_name}' not found."})
            return

        self.current_state = self.states[self.start_state_name]
        logging.info(f"FSM starting at state: {self.current_state.name}")
        self._send_to_client("FSM_STARTED", {"start_state": self.current_state.name})

        # Outer FSM loop: continues as long as FSM is not stopped and has a current state
        while not self._stop_event.is_set() and self.current_state:
            logging.info(f"--- Processing state: {self.current_state.name} ---")
            self._send_to_client("CURRENT_STATE", {"name": self.current_state.name, "is_finish": self.current_state.is_finish_state})

            if self.current_state.action:
                logging.info(f"Executing action for state {self.current_state.name}")
                try:
                    with self._variable_lock: vars_copy = self.variables.copy()
                    self.current_state.action(self, vars_copy) # Pass FSM instance and vars copy
                    self._send_to_client("STATE_ACTION_EXECUTED", {"state_name": self.current_state.name})
                except Exception as e:
                    logging.error(f"Error executing action for state {self.current_state.name}: {e}")
                    self._send_to_client("FSM_ERROR", {"message": f"Action error in state {self.current_state.name}: {str(e)}"})
                    self.stop(); break

            if self._stop_event.is_set(): break

            if self.current_state.is_finish_state:
                logging.info(f"Reached finish state: {self.current_state.name}")
                self._send_to_client("FSM_FINISHED", {"finish_state": self.current_state.name})
                break

            # Inner loop for transition evaluation and execution for the self.current_state.
            # This loop continues until a state transition occurs, FSM stops, or gets stuck.
            # It can be re-entered if a delay is interrupted by _re_evaluate_event.
            while not self._stop_event.is_set():
                self._re_evaluate_event.clear() # Clear before evaluating transitions for this iteration

                # 1. Evaluate transitions to find one to take
                transition_to_take = None
                for t in self.current_state.transitions:
                    with self._variable_lock: vars_copy = self.variables.copy()
                    can_transit = False
                    try:
                        can_transit = t.condition(self, vars_copy) # Pass FSM instance and vars copy
                    except Exception as e:
                        logging.error(f"Error evaluating condition for transition to {t.target_state_name} from {self.current_state.name}: {e}")
                        self._send_to_client("FSM_ERROR", {"message": f"Condition error for transition from {self.current_state.name}: {str(e)}"})
                        self.stop(); break 
                    
                    if self._stop_event.is_set(): break

                    if can_transit:
                        transition_to_take = t
                        break # Found the first (highest priority) valid transition
                
                if self._stop_event.is_set(): break # from this inner transition processing loop

                if not transition_to_take:
                    logging.warning(f"FSM stuck in state {self.current_state.name}: No valid transitions.")
                    self._send_to_client("FSM_STUCK", {"state_name": self.current_state.name})
                    self.stop(); break # Break from inner transition processing loop, FSM will stop

                # 2. A transition_to_take has been selected.
                logging.info(f"Selected transition: {self.current_state.name} -> {transition_to_take.target_state_name}")
                self._send_to_client("TRANSITION_TAKEN", {
                    "from_state": self.current_state.name,
                    "to_state": transition_to_take.target_state_name,
                    "delay": transition_to_take.delay
                })

                if transition_to_take.action:
                    logging.info(f"Executing action for transition: {self.current_state.name} -> {transition_to_take.target_state_name}")
                    try:
                        with self._variable_lock: # Action might read/write live vars
                            transition_to_take.action(self.variables) 
                        self._send_to_client("TRANSITION_ACTION_EXECUTED", {
                            "from_state": self.current_state.name,
                            "to_state": transition_to_take.target_state_name
                        })
                    except Exception as e:
                        logging.error(f"Error executing transition action: {e}")
                        self._send_to_client("FSM_ERROR", {"message": f"Transition action error: {str(e)}"})
                        self.stop(); break # Break from inner transition processing loop

                if self._stop_event.is_set(): break

                # 3. Handle delay for transition_to_take
                if transition_to_take.delay > 0:
                    self._current_delay_target_transition = transition_to_take
                    # transition.delay is in seconds
                    delay_seconds = transition_to_take.delay / 1000.0 # Convert milliseconds to seconds
                    self._current_delay_end_time = time.time() + delay_seconds
                    
                    logging.info(f"Starting delay for {delay_seconds:.2f}s for transition to {transition_to_take.target_state_name}")
                    
                    needs_re_evaluation = False
                    while not self._stop_event.is_set() and time.time() < self._current_delay_end_time:
                        remaining_delay = self._current_delay_end_time - time.time()
                        if remaining_delay <= 0: break # Delay naturally ended

                        wait_duration = min(0.1, remaining_delay) # Check frequently
                        
                        # _re_evaluate_event.wait clears the event if it returns True
                        if self._re_evaluate_event.wait(timeout=wait_duration):
                            if self._stop_event.is_set(): break # Prioritize stop event
                            
                            logging.info(f"Re-evaluation signaled during delay for transition to "
                                         f"{self._current_delay_target_transition.target_state_name}. Restarting transition search for state {self.current_state.name}.")
                            needs_re_evaluation = True
                            break # Exit delay loop to re-evaluate all transitions from current_state
                    
                    # Clear current delay tracking information as this delay attempt is over
                    self._current_delay_target_transition = None
                    self._current_delay_end_time = None

                    if self._stop_event.is_set(): break # Break from inner transition processing loop

                    if needs_re_evaluation:
                        # _re_evaluate_event is already cleared by wait().
                        # Continue to the top of this inner "Transition evaluation..." loop
                        # to re-scan all transitions from self.current_state.
                        continue 

                    # If we are here, delay completed naturally (or was very short) without stop or re-evaluation.
                    # Check if time is up, effectively.
                    if time.time() < self._current_delay_end_time if self._current_delay_end_time else False: # Defensive check, should mean loop exited for other reason
                        logging.warning("Delay loop for transition exited prematurely without re-evaluation or stop signal, before time was up. Re-evaluating.")
                        continue


                    logging.info(f"Delay completed for transition to {transition_to_take.target_state_name}.")
                    # Proceed to change state (handled below this if-block)

                # 4. If delay is zero or completed (and not re-evaluating/stopped), perform the state change.
                # (Current delay tracking already cleared if delay was active)
                if transition_to_take.target_state_name not in self.states:
                    logging.error(f"Target state '{transition_to_take.target_state_name}' not found!")
                    self._send_to_client("FSM_ERROR", {"message": f"Target state '{transition_to_take.target_state_name}' not found."})
                    self.stop(); break # Break from inner transition processing loop

                logging.info(f"Completing transition: {self.current_state.name} -> {transition_to_take.target_state_name}")
                self.current_state = self.states[transition_to_take.target_state_name]
                # Successfully transitioned, break inner loop to process the new current_state in outer loop
                break 
            
            # End of inner "Transition evaluation..." loop.
            # If this loop broke due to stop(), the outer loop's stop_event check will catch it.
            # If it broke due to a state change, the outer loop continues with the new current_state.

        # FSM execution loop has ended
        if self._stop_event.is_set() and not self.current_state.is_finish_state : # only send FSM_STOPPED if not already finished
             # Check if FSM_FINISHED or FSM_STUCK already explains the stop
            is_stuck = (self.current_state and not self.current_state.is_finish_state and
                        not any(t.condition(self, self.variables.copy()) for t in self.current_state.transitions))
            if not is_stuck : # Avoid duplicate FSM_STUCK vs FSM_STOPPED messages if stuck caused stop.
                logging.info("FSM run loop terminated by stop event.")
                self._send_to_client("FSM_STOPPED", {"message": "FSM was stopped."})
        
        self._cleanup()

    def stop(self):
        logging.info("Stop requested for FSM.")
        self._stop_event.set()
        # Also signal re-evaluate to break any current delay wait immediately
        self._re_evaluate_event.set() 

    def _cleanup(self):
        logging.info("FSM cleaning up...")
        # Clear any pending delay info, FSM is stopping.
        self._current_delay_target_transition = None
        self._current_delay_end_time = None

        if self._client_socket:
            try:
                # Avoid sending if socket already seems problematic or FSM ended with specific message
                # self._send_to_client("FSM_SHUTTING_DOWN", {}) # Consider if this is needed
                self._client_socket.shutdown(socket.SHUT_RDWR) 
                self._client_socket.close()
            except (socket.error, AttributeError):
                pass 
            self._client_socket = None
        
        if self._client_handler_thread and self._client_handler_thread.is_alive():
            self._client_handler_thread.join(timeout=1.0) 
            if self._client_handler_thread.is_alive():
                logging.warning("Client handler thread did not terminate gracefully.")
        
        logging.info("FSM has shut down.")