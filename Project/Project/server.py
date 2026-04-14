import socket
import threading
import json

HOST = '0.0.0.0'
PORT = 5050

waiting_host = None
waiting_host_lock = threading.Lock()

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

def relay(src_conn, dst_conn_ref, src_id, src_buf, both_conns_ref):
    try:
        while True:
            msg = recv_line(src_conn, src_buf)
            dst = dst_conn_ref[0]
            if dst:
                if not send_line(dst, msg):
                    break
    except Exception as e:
        print(f"[Player {src_id}] disconnected: {e}")
    finally:
        # Force-close both sides
        for c in both_conns_ref:
            if c:
                try: c.shutdown(socket.SHUT_RDWR)
                except: pass
                try: c.close()
                except: pass
        both_conns_ref[0] = None
        both_conns_ref[1] = None

def start_match(host_conn, host_buf, joiner_conn, joiner_buf):
    print("Match started!")
    if not send_line(host_conn, json.dumps({"player_id": 0})):
        try: host_conn.close()
        except: pass
        try: joiner_conn.close()
        except: pass
        return
    if not send_line(joiner_conn, json.dumps({"player_id": 1})):
        try: host_conn.close()
        except: pass
        try: joiner_conn.close()
        except: pass
        return
    if not send_line(host_conn, json.dumps({"status": "START"})):
        return
    if not send_line(joiner_conn, json.dumps({"status": "START"})):
        return

    conns = [host_conn, joiner_conn]
    host_ref = [host_conn]
    joiner_ref = [joiner_conn]

    t0 = threading.Thread(target=relay, args=(host_conn, joiner_ref, 0, host_buf, conns), daemon=True)
    t1 = threading.Thread(target=relay, args=(joiner_conn, host_ref, 1, joiner_buf, conns), daemon=True)
    t0.start()
    t1.start()
    t0.join()
    t1.join()
    print("Match ended cleanly")

def handle_new_connection(conn, addr):
    global waiting_host
    try:
        conn.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
        buf = [b'']
        role_line = recv_line(conn, buf)
        is_host = '"host"' in role_line

        if is_host:
            with waiting_host_lock:
                # If there's already a waiting host, kick the old one
                if waiting_host is not None:
                    old_conn, _ = waiting_host
                    print("Replacing stale waiting host")
                    try: old_conn.close()
                    except: pass
                waiting_host = (conn, buf)
            print(f"[HOST] {addr} waiting for joiner...")
        else:
            with waiting_host_lock:
                if waiting_host is None:
                    print(f"[JOINER] {addr} arrived but no host waiting!")
                    try: conn.close()
                    except: pass
                    return
                host_conn, host_buf = waiting_host
                waiting_host = None
            print(f"[JOINER] {addr} matched with host")
            # Verify host still alive
            try:
                host_conn.send(b'')
            except:
                print("Host died before joiner arrived")
                try: host_conn.close()
                except: pass
                try: conn.close()
                except: pass
                return
            start_match(host_conn, host_buf, conn, buf)
    except Exception as e:
        print(f"Connection error from {addr}: {e}")
        try: conn.close()
        except: pass

def main():
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind((HOST, PORT))
    server.listen()
    print(f"ASTEROID 3D Server running on port {PORT}")
    while True:
        try:
            conn, addr = server.accept()
            t = threading.Thread(target=handle_new_connection, args=(conn, addr), daemon=True)
            t.start()
        except Exception as e:
            print(f"Accept error: {e}")

main()