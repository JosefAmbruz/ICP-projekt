import socket
import json
import threading
import time
import sys

FSM_HOST = 'localhost'
FSM_PORT = 65432 # Make sure this matches the port in fsm_runner.py

# Event to signal threads to stop
stop_event = threading.Event()

def parse_value(value_str):
    """Tries to parse a string into int, float, bool, or keeps it as string."""
    value_str = value_str.strip()
    if value_str.lower() == 'true':
        return True
    if value_str.lower() == 'false':
        return False
    try:
        return int(value_str)
    except ValueError:
        try:
            return float(value_str)
        except ValueError:
            return value_str # Keep as string if other parses fail

def receive_messages(sock):
    """
    Receives messages from the FSM server and prints them.
    Runs in a separate thread.
    """
    buffer = ""
    try:
        while not stop_event.is_set():
            try:
                # Set a timeout so the loop can check stop_event
                sock.settimeout(0.5)
                data = sock.recv(4096)
                if not data:
                    print("\n[Client] Server closed the connection.")
                    break
                
                buffer += data.decode('utf-8')
                
                while '\n' in buffer:
                    message_str, buffer = buffer.split('\n', 1)
                    if not message_str.strip():
                        continue
                    try:
                        message = json.loads(message_str)
                        print(f"[FSM -> Client] {json.dumps(message, indent=2)}")
                    except json.JSONDecodeError:
                        print(f"[Client] Received invalid JSON: {message_str}")

            except socket.timeout:
                continue # Allows checking stop_event
            except (socket.error, ConnectionResetError) as e:
                print(f"\n[Client] Socket error: {e}. Disconnecting.")
                break
            except Exception as e:
                print(f"\n[Client] Error receiving message: {e}")
                break
    finally:
        print("[Client] Receiver thread stopping.")
        stop_event.set() # Signal other threads (like sender) to stop

def send_messages(sock):
    """
    Allows the user to send commands to the FSM.
    """
    print("\n[Client] Type commands to send to FSM:")
    print("  'set <var_name> <var_value>' (e.g., 'set counter 5', 'set force_finish true')")
    print("  'stopfsm' to tell the FSM to stop")
    print("  'quit' to exit this client")
    print("-----------------------------------------")

    try:
        while not stop_event.is_set():
            try:
                # Use a timeout for input to allow checking stop_event periodically
                # This is tricky with blocking input(), so we'll rely on user typing 'quit'
                # or receiver thread signaling stop.
                # A more robust solution might use select() on stdin for non-blocking input.
                
                # For simplicity, input() will block. User needs to press Enter.
                if sys.stdin.isatty(): # Check if input is from a terminal
                    command_str = input("> ")
                else: # If input is piped, read line by line, or just wait for stop_event
                    time.sleep(0.1) # Don't busy-wait if not interactive
                    if stop_event.is_set(): break
                    continue


                if stop_event.is_set(): # Check after input attempt
                    break

                if not command_str.strip():
                    continue

                parts = command_str.strip().lower().split(maxsplit=2)
                command = parts[0]
                message_to_send = None

                if command == "quit":
                    print("[Client] Quitting...")
                    break
                elif command == "stopfsm":
                    message_to_send = {"type": "STOP_FSM", "payload": {}}
                elif command == "set" and len(parts) == 3:
                    var_name = parts[1]
                    var_value_str = parts[2]
                    var_value = parse_value(var_value_str)
                    message_to_send = {
                        "type": "SET_VARIABLE",
                        "payload": {"name": var_name, "value": var_value}
                    }
                else:
                    print(f"[Client] Unknown command or incorrect format: '{command_str}'")
                    continue

                if message_to_send:
                    try:
                        sock.sendall((json.dumps(message_to_send) + "\n").encode('utf-8'))
                        print(f"[Client -> FSM] Sent: {json.dumps(message_to_send)}")
                    except socket.error as e:
                        print(f"[Client] Error sending message: {e}. Connection may be lost.")
                        break
            except EOFError: # Happens if input is piped and ends
                print("[Client] EOF received, quitting sender.")
                break
            except KeyboardInterrupt:
                print("\n[Client] Ctrl+C detected in sender, quitting.")
                break


    finally:
        print("[Client] Sender loop stopping.")
        stop_event.set() # Ensure other threads know to stop

if __name__ == "__main__":
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        print(f"[Client] Attempting to connect to FSM at {FSM_HOST}:{FSM_PORT}...")
        client_socket.connect((FSM_HOST, FSM_PORT))
        print("[Client] Connected to FSM.")

        receiver_thread = threading.Thread(target=receive_messages, args=(client_socket,), daemon=True)
        receiver_thread.start()

        # Run sender in the main thread or create another thread for it
        # For simplicity, run in main thread. It will block on input().
        send_messages(client_socket)

    except socket.error as e:
        print(f"[Client] Could not connect to FSM: {e}")
    except KeyboardInterrupt:
        print("\n[Client] Ctrl+C detected, shutting down client.")
    finally:
        stop_event.set() # Signal all threads to stop
        if 'receiver_thread' in locals() and receiver_thread.is_alive():
            receiver_thread.join(timeout=1.0) # Wait for receiver to finish

        print("[Client] Closing socket.")
        client_socket.close()
        print("[Client] Exited.")