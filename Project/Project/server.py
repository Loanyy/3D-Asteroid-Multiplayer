import socket
import threading
import json

HOST = '0.0.0.0'
PORT = 5050

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.bind((HOST, PORT))
server.listen()

jucatori = []
date_jucatori = {}

def handle_client(conn, player_id):
    print(f"Player {player_id} conectat!")
    
  
    conn.send(json.dumps({"player_id": player_id}).encode('utf-8'))

    while True:
        try:
          
            date_brute = conn.recv(4096).decode('utf-8')
            date = json.loads(date_brute)

            
            date_jucatori[player_id] = date

            
            alt_id = 1 if player_id == 0 else 0
            if alt_id in date_jucatori:
                alt_conn = jucatori[alt_id]
                alt_conn.send(json.dumps(date_jucatori[player_id]).encode('utf-8'))

        except:
            print(f"Player {player_id} deconectat!")
            jucatori[player_id] = None
            break

def start():
    print("Server pornit astept jucatori...")
    while len(jucatori) < 2:
        conn, addr = server.accept()
        player_id = len(jucatori)
        jucatori.append(conn)
        
        thread = threading.Thread(target=handle_client, args=(conn, player_id))
        thread.start()

    print("Ambii jucatori conectati - jocul incepe!")
    for conn in jucatori:
        conn.send(json.dumps({"status": "START"}).encode('utf-8'))

start()
