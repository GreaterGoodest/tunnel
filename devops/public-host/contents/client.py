import sys
import socket

LOCAL_ADDR = '127.0.0.1'
LOCAL_PORT = 1336
REMOTE_ADDR = '172.21.1.3'
REMOTE_PORT = 1337

KB = 1024
DATA_SIZE = 1 * KB

def listen_for_server() -> socket.socket:
    """Listen for remote server connection."""
    remote_listener = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    remote_listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    try:
        remote_listener.bind((REMOTE_ADDR, REMOTE_PORT))
    except:
        print('Unable to bind listener')
        return None
    remote_listener.listen(5)
    local_conn, _ = remote_listener.accept()
    print('Received remote connection')
    return local_conn


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
    remote_conn = listen_for_server()
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