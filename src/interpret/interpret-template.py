from automata_core import Automaton, State, Transition
import socket

# Klient (cpp) posílá stringy obsahující python kód
# Kód je vyhodnocen dynamicky na serveru
# TODO: Zpátky se eventuálně bude posílat nejspíš JSON 
#  obsahující vyhodnocené proměnné, inputy a outputy
# - Timeouty bych řešil asi na klientovi - pokud by to ale měly být proměnné, tak to nepůjde
# - Guarding podmínky přechodů bych vyhodnocoval separátně,
#    klient pošle požadavek s podmínkou, vrátí se TRUE/FALSE ze serveru
# -> Guarding podmínky a timeout řešit dohromady, ale vlastní zprávou
#     která když serveru přijde, vyhodnotí se podmínka a spustí se časovač
#     Jakmile časovač doběhne, odešle se zpráva s TRUE/FALSE


HOST = '127.0.0.1'
PORT = 9898 # TODO: port by měl být změnitelný - jak?

global_env = {"__builtins__": {"dir" : dir}} # tady můžu napsat podporované funkce


def handle_request(data):
    if data == "STATUS":
        return "Debugger is running"
    elif data.startswith("EVAL "):
        code = data[5:]
        try:
            compiled_code = compile(code, '<debugger>', 'exec')
            exec(compiled_code, global_env, global_env)
            return f"Executed. Locals: {global_env}"
        except Exception as e:
            return f"Error: {str(e)}"
    else:
        return "Unknown command"
    
def exec_command(cmd):
    globalParam = {}
    localsParam = {}

    code = compile(cmd, "<string>", "exec")
    return exec(code, globalParam, localsParam)

def main():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((HOST, PORT))
        s.listen()
        print(f"Debugger is running and listening on {HOST}:{PORT}")

        while True:
            conn, addr = s.accept()
            with conn:
                print(f"Connected by {addr}")
                while True:
                    data = conn.recv(1024).decode('utf-8')
                    if not data:
                        break
                    print(f"Received: {data}")
                    response = handle_request(data)
                    conn.sendall(response.encode('utf-8'))

if __name__ == "__main__":
    main()
