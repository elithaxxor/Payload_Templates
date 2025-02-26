import socket
import threading
import os
import logging
import time

BUFFER_SIZE = 8192
CLIENT_PORT = 22223
logging.basicConfig(filename='server.log', level=logging.DEBUG, format='%(asctime)s - %(levelname)s - %(message)s')

def init_logging():
    logging.basicConfig(
        filename='server.log',
        level=logging.DEBUG,
        format='%(asctime)s - %(levelname)s - %(message)s'
    )
    logging.info("Logging initialized")


def handle_client(client_socket, addr):
    try:
        logging.info(f"Connected to {addr}")
        print(f"[+] Connected to {addr}")

        ''' #1. Receive file name and size from client '''
        file_name = client_socket.recv(100).decode('utf-8')
        file_size = int(client_socket.recv(100).decode('utf-8'))
        send_count = 0
        send_start = time.time()

        logging.info(f"Sending file: {file_name} of size: {file_size} bytes")
        print("[!] Sending file: {file_name} of size: {file_size} bytes")


        ''' #2. Open and read as binary for parsing, then send to client '''
        with open(file_name, "rb") as file:
            while True:
                data = file.read(BUFFER_SIZE)
                if not data:
                    logging.info("File transfer complete")
                    print("File transfer complete")
                    break
                client_socket.sendall(data)
                send_count += len(data)
                logging.info(f"Sent {send_count} of {file_size} bytes")
                print("[!] Sent {send_count} of {file_size} bytes")

        ''' #3. Close the file and socket '''
        send_end = time.time()
        logging.info(f"[+] Transfer complete in {send_end - send_start} seconds")
        print("[+] Transfer complete in {send_end - send_start} seconds")

    except Exception as e:
        logging.error(f"Error: {e}")
        print("Error: ", e)

    finally:
        client_socket.close()
        print("[-] Client socket closed")
        logging.info("Client socket closed")


'''The main function sets up the server socket, binds it to the hostname and port, and starts listening for incoming connections. When a client connects, a new thread is created to handle the client using the handle_client function. This allows the server to handle multiple clients concurrently.'''
def main():
    ''' #1. Create server socket and bind to host and port '''
    init_logging()
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind((socket.gethostname(), CLIENT_PORT))
    server_socket.listen(5)
    logging.info(f"Server listening on port {CLIENT_PORT}")
    print("[!] Server is running. Waiting for connections...")
    print("[!]  Accept clients and start a new thread for each one, allowing multiple clients to connect simultaneously ")
    #time.sleep(1)
    '''#2. Accept clients and start a new thread for each one, allowing multiple clients to connect simultaneously '''

    '''# The `start` method in the code is used to start the execution of a thread. In the context of the provided code snippet, when `client_handler.start()` is called, it initiates the execution of the thread represented by `client_handler`. This allows the server to handle multiple clients concurrently by creating a new thread for each incoming client connection.'''
    counter = 0
    while True:
        print("[-] Attempting to start new thread #", counter)
        client_socket, addr = server_socket.accept()
        client_handler = threading.Thread(target=handle_client, args=(client_socket, addr))
        logging.info(f"Server started New thread started for {addr}")

        if client_handler.start() != True:
            print(f"[-] Server started New thread started for {addr}")
            print(".. checking if handle starter is alive: ", client_handler.is_alive())
            counter = counter + 1
            client_handler.start()

        logging.info(f"Server started New thread started for {addr} and count is: {counter}")
        print(f"[+] Server started New thread started for {addr} and count is: {counter}")
        client_handler.start()



if __name__ == "__main__":
    logging.info("Starting server...")
    print("[!] Update to use threading for multiple clients on server. The main function creates a server socket, binds to the host's name and CLIENT_PORT, \nlistens for connections. \nThen in a loop, it accepts clients, starts a new thread for each, passing the client socket and address to handle_client")
    print("\n\n[!] Server is running. Waiting for connections...")
    main()



