import sys
import socket

LOCAL_ADDR = '127.0.0.1'
LOCAL_PORT = 1336
REMOTE_ADDR = '127.0.0.1'
REMOTE_PORT = 1337

KB = 1024
DATA_SIZE = 1 * KB

def connect_to_server() -> socket.socket:
    """Connect to remote server for tunneling."""
    remote_conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        remote_conn.connect((REMOTE_ADDR, REMOTE_PORT))
    except ConnectionRefusedError:
        print(f'Server at {REMOTE_ADDR}:{REMOTE_PORT} is not available.')
        return None
    print(f'Successfullly connect to remote server at {REMOTE_ADDR}:{REMOTE_PORT}.')
    return remote_conn

def listen_local() -> socket.socket:
    """Listen for local data to tunnel."""
    local_listener = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    local_listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    try:
        local_listener.bind((LOCAL_ADDR, LOCAL_PORT))
    except:
        print('Unable to bind listener')
        return None
    local_listener.listen(5)
    local_conn, _ = local_listener.accept()
    print('Received local connection')
    return local_conn

def tunnel_loop(local_conn: socket.socket, remote_conn: socket.socket) -> None:
    """Main tunnel traffic exchange loop."""
    data = None
    while True:
        try:
            data = local_conn.recv(DATA_SIZE)
        except BlockingIOError:
            pass
        if data:
            remote_conn.send(data)
        data = None
        try:
            data = remote_conn.recv(DATA_SIZE)
        except BlockingIOError:
            pass
        if data:
            local_conn.send(data)
        data = None

def init(dest: int):
    """Tunnel setup and initialization."""
    remote_conn = connect_to_server()
    if not remote_conn:
        return
    remote_conn.setblocking(0)

    local_conn = listen_local()
    if not local_conn:
        return
    local_conn.setblocking(0)
    tunnel_loop(local_conn, remote_conn)


if __name__ == "__main__":
    dest_select = 1
    if len(sys.argv) > 1:
        dest_select = sys.argv[1]
    init(dest_select)