import socket
import threading
import json

HOST = '0.0.0.0'
PORT = 5050

def recv_line(conn, buf):
    while True:
        idx = buf[0].find(b'\n')
        if idx >= 0:
            line = buf[0][:idx].decode('utf-8', errors='ignore')
            buf[0] = buf[0][idx+1:]
            return line
        chunk = conn.recv(4096)
        if not chunk:
            raise ConnectionError("disconnected")
        buf[0] += chunk

def send_line(conn, msg):
    try:
        conn.sendall((msg + '\n').encode('utf-8'))
        return True
    except:
        return False

def handle_client(conn, player_id, clients, lock, buf):
    print(f"[Player {player_id}] handler started")
    try:
        while True:
            msg = recv_line(conn, buf)
            other_id = 1 - player_id
            with lock:
                other = clients[other_id]
            if other:
                if not send_line(other, msg):
                    break
    except Exception as e:
        print(f"[Player {player_id}] disconnected: {e}")
    finally:
        with lock:
            clients[player_id] = None
        try:
            conn.shutdown(socket.SHUT_RDWR)
        except:
            pass
        try:
            conn.close()
        except:
            pass
        # Notify the other player so they detect disconnect immediately
        with lock:
            other = clients[1 - player_id]
        if other:
            try:
                other.shutdown(socket.SHUT_RDWR)
            except:
                pass
            try:
                other.close()
            except:
                pass
            with lock:
                clients[1 - player_id] = None

def run_match(server):
    clients = [None, None]
    lock = threading.Lock()
    buf0 = [b'']
    buf1 = [b'']

    print("\n--- Waiting for HOST ---")
    conn0, addr0 = server.accept()
    conn0.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
    clients[0] = conn0
    print(f"[HOST] connected: {addr0}")
    if not send_line(conn0, json.dumps({"player_id": 0})):
        print("Host disconnected before start")
        try: conn0.close()
        except: pass
        return

    print("--- Waiting for JOINER ---")
    while True:
        try:
            conn1, addr1 = server.accept()
            conn1.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
            # Check if host is still alive
            try:
                conn0.send(b'')
            except:
                print("Host disconnected while waiting for joiner")
                try: conn1.close()
                except: pass
                try: conn0.close()
                except: pass
                return
            clients[1] = conn1
            print(f"[JOINER] connected: {addr1}")
            break
        except Exception as e:
            print(f"Accept error: {e}")
            return

    if not send_line(conn1, json.dumps({"player_id": 1})):
        print("Joiner disconnected before start")
        return

    print("Both connected - START!")
    if not send_line(conn0, json.dumps({"status": "START"})):
        return
    if not send_line(conn1, json.dumps({"status": "START"})):
        return

    t0 = threading.Thread(target=handle_client, args=(conn0, 0, clients, lock, buf0), daemon=True)
    t1 = threading.Thread(target=handle_client, args=(conn1, 1, clients, lock, buf1), daemon=True)
    t0.start()
    t1.start()
    t0.join()
    t1.join()
    print("Match cleaned up")

def main():
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind((HOST, PORT))
    server.listen()
    print(f"ASTEROID 3D Server running on port {PORT}")
    while True:
        try:
            run_match(server)
        except Exception as e:
            print(f"Match error: {e}")
        print("Ready for next match...\n")

main()