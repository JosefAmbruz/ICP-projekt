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
            condition (callable, optional): A function that takes a dictionary of FSM variables
                                           and returns True if the transition can be taken,
                                           False otherwise. Defaults to always True.
            action (callable, optional): A function to execute when this transition is taken.
                                        It takes a dictionary of FSM variables.
            delay (float, optional): Time in seconds to wait before completing the transition.
                                     Defaults to 0.0.
        """
        self.target_state_name = target_state_name
        self.condition = condition if callable(condition) else lambda variables: True
        self.action = action
        self.delay = delay

    def __repr__(self):
        return f"<Transition to '{self.target_state_name}' delay={self.delay}s>"

class State:
    def __init__(self, name, action=None, is_start_state=False, is_finish_state=False):
        """
        Represents a state in the FSM.

        Args:
            name (str): The unique name of the state.
            action (callable, optional): A function to execute upon entering this state.
                                        It takes a dictionary of FSM variables.
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


    def get_variable(self, name, default=None):
        with self._variable_lock:
            return self.variables.get(name, default)

    def _send_to_client(self, message_type, payload=None):
        if self._client_socket:
            try:
                message = {"type": message_type, "payload": payload or {}}
                self._client_socket.sendall((json.dumps(message) + "\n").encode('utf-8'))
                # logging.debug(f"Sent to client: {message}")
            except (socket.error, BrokenPipeError) as e:
                logging.error(f"Error sending message to client: {e}. Client might have disconnected.")
                self._handle_disconnection()


    def _handle_client_messages(self):
        """Runs in a separate thread to listen for client messages."""
        buffer = ""
        try:
            while not self._stop_event.is_set() and self._client_socket:
                try:
                    # Set a timeout so the loop can check _stop_event periodically
                    self._client_socket.settimeout(0.5)
                    data = self._client_socket.recv(1024)
                    if not data:
                        logging.info("Client disconnected gracefully.")
                        self._handle_disconnection()
                        break
                    
                    buffer += data.decode('utf-8')
                    
                    # Process all complete JSON messages in the buffer
                    while '\n' in buffer:
                        message_str, buffer = buffer.split('\n', 1)
                        if not message_str.strip(): # Skip empty lines
                            continue
                        try:
                            message = json.loads(message_str)
                            logging.info(f"Received from client: {message}")
                            if message.get("type") == "SET_VARIABLE":
                                var_name = message.get("payload", {}).get("name")
                                var_value = message.get("payload", {}).get("value")
                                if var_name is not None:
                                    self.set_variable(var_name, var_value) # This will acquire lock
                            elif message.get("type") == "STOP_FSM":
                                logging.info("Received STOP_FSM command from client.")
                                self.stop() # Signal main loop to stop
                            # Add other message types here if needed
                        except json.JSONDecodeError:
                            logging.warning(f"Invalid JSON received from client: {message_str}")
                        except Exception as e:
                            logging.error(f"Error processing client message: {e}")
                            
                except socket.timeout:
                    continue # Timeout allows checking _stop_event
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
            self._send_to_client("FSM_ERROR", {"message": "Client disconnected or connection lost."})
            self.stop() # Stop the FSM if client disconnects
            try:
                self._client_socket.close()
            except socket.error:
                pass # Already closed or error
            self._client_socket = None


    def connect_to_client(self, host='localhost', port=12345):
        """Connects to the client application."""
        server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1) # Allow quick reuse of address
        try:
            server_socket.bind((host, port))
            server_socket.listen(1)
            logging.info(f"FSM Server listening on {host}:{port}")
            print(f"FSM Server: Waiting for a client connection on {host}:{port}...")
            
            # Set a timeout for accept() so it doesn't block indefinitely if we need to shut down
            server_socket.settimeout(1.0) 
            while not self._stop_event.is_set():
                try:
                    conn, addr = server_socket.accept()
                    self._client_socket = conn
                    self._client_address = addr
                    logging.info(f"Client connected from {addr}")
                    print(f"FSM Server: Client connected from {addr}")
                    self._send_to_client("FSM_CONNECTED", {"message": "Successfully connected to FSM."})
                    
                    # Start client message handler thread
                    self._client_handler_thread = threading.Thread(target=self._handle_client_messages, daemon=True)
                    self._client_handler_thread.start()
                    break # Exit accept loop once connected
                except socket.timeout:
                    continue # Loop back and check _stop_event
                except Exception as e:
                    logging.error(f"Error accepting connection: {e}")
                    self.stop() # Signal FSM to stop if connection fails badly
                    break
        
        except Exception as e:
            logging.error(f"Could not start FSM server: {e}")
            self.stop() # Signal FSM to stop
        finally:
            server_socket.close() # Close the listening server socket

        if not self._client_socket and not self._stop_event.is_set():
            logging.error("Failed to connect to any client. FSM cannot run.")
            self.stop() # Ensure FSM stops if no client connects


    def run(self):
        """Runs the FSM interpreter."""
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

        while not self._stop_event.is_set() and self.current_state:
            self._send_to_client("CURRENT_STATE", {"name": self.current_state.name, "is_finish": self.current_state.is_finish_state})
            logging.info(f"Current state: {self.current_state.name}")

            if self.current_state.action:
                logging.info(f"Executing action for state {self.current_state.name}")
                try:
                    # Pass a copy of variables to avoid modification issues if action is slow
                    # and variables change due to client messages
                    with self._variable_lock:
                        vars_copy = self.variables.copy()
                    self.current_state.action(vars_copy)
                    self._send_to_client("STATE_ACTION_EXECUTED", {"state_name": self.current_state.name})
                except Exception as e:
                    logging.error(f"Error executing action for state {self.current_state.name}: {e}")
                    self._send_to_client("FSM_ERROR", {"message": f"Action error in state {self.current_state.name}: {str(e)}"})
                    self.stop() # Critical error, stop FSM
                    break


            if self.current_state.is_finish_state:
                logging.info(f"Reached finish state: {self.current_state.name}")
                self._send_to_client("FSM_FINISHED", {"finish_state": self.current_state.name})
                break

            if self._stop_event.is_set(): break # Check before finding next transition

            next_transition_found = False
            for transition in self.current_state.transitions:
                # Critical section for reading variables for condition check
                with self._variable_lock:
                    vars_copy = self.variables.copy()
                
                can_transit = False
                try:
                    can_transit = transition.condition(vars_copy)
                except Exception as e:
                    logging.error(f"Error evaluating condition for transition to {transition.target_state_name} from {self.current_state.name}: {e}")
                    self._send_to_client("FSM_ERROR", {"message": f"Condition error for transition from {self.current_state.name}: {str(e)}"})
                    self.stop() # Critical error
                    break # Break from transition loop

                if self._stop_event.is_set(): break # Check after condition evaluation
                
                if can_transit:
                    logging.info(f"Condition met for transition: {self.current_state.name} -> {transition.target_state_name}")
                    self._send_to_client("TRANSITION_TAKEN", {
                        "from_state": self.current_state.name,
                        "to_state": transition.target_state_name,
                        "delay": transition.delay
                    })

                    if transition.action:
                        logging.info(f"Executing action for transition: {self.current_state.name} -> {transition.target_state_name}")
                        try:
                            with self._variable_lock: # Action might read/write vars
                                transition.action(self.variables) # Pass live variables
                            self._send_to_client("TRANSITION_ACTION_EXECUTED", {
                                "from_state": self.current_state.name,
                                "to_state": transition.target_state_name
                            })
                        except Exception as e:
                            logging.error(f"Error executing transition action: {e}")
                            self._send_to_client("FSM_ERROR", {"message": f"Transition action error: {str(e)}"})
                            self.stop() # Critical error
                            break # Break from transition loop
                    
                    if self._stop_event.is_set(): break # Check after transition action

                    if transition.delay > 0:
                        logging.info(f"Delaying transition for {transition.delay} seconds...")
                        # Sleep in small chunks to check _stop_event more frequently
                        end_time = time.time() + transition.delay
                        while time.time() < end_time and not self._stop_event.is_set():
                            time.sleep(min(0.1, end_time - time.time())) # Sleep for at most 0.1s or remaining time
                        if self._stop_event.is_set():
                            logging.info("FSM stop requested during transition delay.")
                            break # Break from transition loop

                    if self._stop_event.is_set(): break # Final check before state change

                    if transition.target_state_name not in self.states:
                        logging.error(f"Target state '{transition.target_state_name}' not found!")
                        self._send_to_client("FSM_ERROR", {"message": f"Target state '{transition.target_state_name}' not found."})
                        self.stop()
                        break # Break from transition loop

                    self.current_state = self.states[transition.target_state_name]
                    next_transition_found = True
                    break # Move to the next state iteration
            
            if self._stop_event.is_set(): break # Check after iterating all transitions

            if not next_transition_found and not self.current_state.is_finish_state:
                logging.warning(f"FSM stuck in state {self.current_state.name}: No valid transitions.")
                self._send_to_client("FSM_STUCK", {"state_name": self.current_state.name})
                break # End FSM

        if self._stop_event.is_set():
            logging.info("FSM run loop terminated by stop event.")
            self._send_to_client("FSM_STOPPED", {"message": "FSM was stopped."})
        
        self._cleanup()

    def stop(self):
        """Signals the FSM to stop its execution and client handling."""
        logging.info("Stop requested for FSM.")
        self._stop_event.set()

    def _cleanup(self):
        logging.info("FSM cleaning up...")
        if self._client_socket:
            try:
                # Inform client FSM is shutting down if not already finished/stuck/error
                if not self.current_state or not self.current_state.is_finish_state: # Check if FSM ended normally
                     # Avoid sending if already sent FSM_FINISHED, FSM_STUCK or FSM_ERROR that led to stop
                    pass # Messages like FSM_STOPPED or FSM_ERROR would have been sent
                self._client_socket.shutdown(socket.SHUT_RDWR) # Gracefully shutdown
                self._client_socket.close()
            except (socket.error, AttributeError):
                pass # Socket might already be closed or None
            self._client_socket = None
        
        if self._client_handler_thread and self._client_handler_thread.is_alive():
            self._client_handler_thread.join(timeout=1.0) # Wait for thread to finish
            if self._client_handler_thread.is_alive():
                logging.warning("Client handler thread did not terminate gracefully.")
        
        logging.info("FSM has shut down.")