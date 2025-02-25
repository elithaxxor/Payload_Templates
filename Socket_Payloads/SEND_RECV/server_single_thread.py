import socket as SOCKET_
import time, os, logging
from numpy.lib.format import BUFFER_SIZE

BUFFER_SIZE = 8192
CLIENT_PORT = 22223
CLIENT_HOSTNAME = SOCKET_.gethostname()
CLIENT_SOCK = SOCKET_.socket(SOCKET_.AF_INET, SOCKET_.SOCK_STREAM)
logging.basicConfig(filename='server.log', level=logging.DEBUG, format='%(asctime)s - %(levelname)s - %(message)s')

def main():
    try:
        CLIENT_HOSTNAME = SOCKET_.gethostname()
        CLIENT_IP = SOCKET_.gethostbyname(CLIENT_HOSTNAME)
        CLIENT_SOCK = SOCKET_.socket(SOCKET_.AF_INET, SOCKET_.SOCK_STREAM)

        logging.info(f"Connected to {CLIENT_HOSTNAME} with IP {CLIENT_IP} at port: {CLIENT_PORT}")
        logging.info(f"Initiating connection..{type(CLIENT_SOCK)} + {CLIENT_SOCK}")
        logging.info(f"{CLIENT_SOCK.getsockname()}")

        print(f"[+] connected to {CLIENT_HOSTNAME} with IP {CLIENT_IP} at port: {CLIENT_PORT}")
        print(f"[+] Initiating connection..{type(CLIENT_SOCK)} + {CLIENT_SOCK}")

        CLIENT_SOCK.bind((CLIENT_IP, CLIENT_PORT))
        CLIENT_SOCK.listen(5)
        print(f"[+] {CLIENT_SOCK.getsockname()}")

        file_name = input("[!] Enter File Name: ")
        file_size = os.path.getsize(file_name)

        client, addr = CLIENT_SOCK.accept()
        print(f"[+] connected to {client}, {addr}")

        client.send(file_name.encode())
        client.send(str(file_size).encode())

        send_count = 0
        send_start = time.time()

        try:
            with open(file_name, "rb") as file:
                while True:
                    data = file.read(BUFFER_SIZE)
                    if not data:
                        break
                    client.sendall(data)
                    send_count += len(data)
                    logging.info(f"Sent {send_count} of {file_size} bytes")
        except IOError as e:
            logging.error(f"File I/O error: {e}")

        send_end = time.time()
        logging.info(f"Transfer complete in {send_end - send_start} seconds")

    except Exception as e:
        logging.error(f"Error: {e}")
        print(f"[ERROR] Socket error: {e}")

    finally:
        if 'CLIENT_SOCK' in locals():
            CLIENT_SOCK.close()
            print("[!] Socket closed")

        if 'client' in locals():
            client.close()
            print("[!] Client socket closed")

if __name__ == "__main__":
    main()