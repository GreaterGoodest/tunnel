import sys
import socket

LOCAL_ADDR = '127.0.0.1'
LOCAL_PORT = 1336
REMOTE_ADDR = '127.0.0.1'
REMOTE_PORT = 1337

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

def listen_local():
    """Listen for local data to tunnel."""

def tunnel_loop(remote_conn: socket.socket):
    """Main tunnel traffic exchange loop."""

def init(dest: int):
    """Tunnel setup and initialization."""
    remote_conn = connect_to_server()
    if not remote_conn:
        return
    listen_local()
    tunnel_loop(remote_conn)


if __name__ == "__main__":
    dest_select = 1
    if len(sys.argv) > 1:
        dest_select = sys.argv[1]
    init(dest_select)