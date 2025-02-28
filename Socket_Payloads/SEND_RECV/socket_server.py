import json 
import socket
import threading
import os
import logging
import time
import struct
import signal

BUFFER_SIZE = 16384
CLIENT_PORT = 22223
MAX_CONNECTIONS = 50
TIMEOUT = 30

# Single logging configuration
logging.basicConfig(
    filename='server.log',
    level=logging.DEBUG,
    format='%(asctime)s - %(levelname)s - %(message)s',
    filemode='a'
)

# Connection limiter
CONNECTION_POOL = threading.BoundedSemaphore(MAX_CONNECTIONS)

def send_directory_listing(client_socket):
    """Send list of available files to client"""
    try:
        # Get list of files (excluding directories)
        try:
            files = [f for f in os.listdir('.') if os.path.isfile(f)]
        except Exception as e:
            print(f"Error listing files: {e}")
            return
        print("[+] Files available \n", files)
        # Create formatted listing
        listing = "\n".join([
            f"{file} - {os.path.getsize(file)} bytes" 
            for file in files
        ]) or "No files available"
        
        print(f"[+] {client_socket} requested Listings available  \n", listing)
        logging.info(f"[+] {client_socket} requested Listings available")

        # Send listing with header
        header = struct.pack('Q', len(listing))
        client_socket.sendall(header)
        client_socket.sendall(listing.encode())
        print("[+] Sent directory to client: ", client_socket)
        logging.info("Sent directory listing to client")
    except Exception as e:
            logging.error(f"Directory listing error: {str(e)}")
            print("[-] Directory listing error occurred, sending blank struct.. : error: \n", e)
            client_socket.sendall(struct.pack('Q', 0))  # Send empty listing
            
            
def handle_clients1(client_socket, addr):
    try:
        client_socket.settimeout(TIMEOUT)
        logging.info(f"Connection from {addr}")
        print(f"[+] Connected to {addr}")
        
        # 4BYTE Commands from client to server
        command = client_socket.recv(4).decode() 
        print(f"[!] Command {command} called by: {addr}")
        logging.info(f"[!] Command {command} called by: {addr}")
        
        if command == "EXIT":
            client_socket.close()
            return

        if command == "LIST":
            files = os.listdir(os.getcwd()) # UNSAFE AND SHOULD REMOVE
            client_socket.sendall(str(files).encode()) # UNSAFE AND SHOULD REMOVE
            send_directory_listing(client_socket) # SAFE AND SHOULD KEEP 
            print(f"Files: {files}")
            return
        
        elif command == "CHAT":
            print("lets talk: ")
            return 
       
            # This part of the code is handling the case when the client sends a command 'FILE' to the server. Here's a breakdown of what the code does:
        elif command == 'FILE':
            # Receive file name length first (fixed size header)
            name_len_header = client_socket.recv(8)
            name_len = struct.unpack('Q', name_len_header)[0]
            
            # Now receive the filename
            file_name_bytes = client_socket.recv(name_len)
            file_name = file_name_bytes.decode('utf-8')
            
            # Receive file size
            size_header = client_socket.recv(8)
            file_size = struct.unpack('Q', size_header)[0]
            
            # Security: sanitize filename
            safe_file_name = os.path.basename(file_name)
            safe_file_path = os.path.join(os.getcwd(), safe_file_name)
            
            print(f"[+] Safe File Name: {safe_file_name}")
            print(f"[+] Safe file path: {safe_file_path}")
            print(f"[!] File Name: {file_name} Size: {file_size}")
            logging.info(f"[+] Safe File Name: {safe_file_name}, Size: {file_size} bytes")
            
            if not os.path.exists(file_name):
                error_msg = f"File {file_name} not found"
                logging.info(f"[-] {error_msg}")
                raise FileNotFoundError(error_msg)

            logging.info(f"Sending {file_name}, {file_size} bytes")
            print(f"[*] Transferring {file_name}, {file_size} bytes")
            
            bytes_sent = 0
            start_time = time.time()

            with open(file_name, "rb") as f:
                while True:
                    data = f.read(BUFFER_SIZE)
                    if not data:
                        print(f"[+] Sent {bytes_sent}/{file_size} bytes")
                        break
                    client_socket.sendall(data)
                    bytes_sent += len(data)

                    # Progress logging
                    if bytes_sent % (BUFFER_SIZE * 10) == 0:  # Log every 10 buffers
                        print(f"[!] Sent {bytes_sent}/{file_size} bytes")

            duration = time.time() - start_time
            logging.info(f"[+] Transfer complete in {duration:.2f}s")
            print(f"[+] Transfer completed in {duration:.2f} seconds")
            
        else:
            logging.error(f"Invalid command: {command}")
            print(f"[-] Invalid command: {command}... try again.")
            try:
                client_socket.send("<ERROR>Invalid command".encode())
            except Exception as each_error:
                print(f"[-] Error in sending error: {each_error}")  
                logging.error(f"[-] Error in sending error: {each_error}")
                
    except Exception as e:
        logging.error(f"Error with {addr}: {str(e)}")
        print(f"[-] Error: {str(e)}")
        try:
            client_socket.send(f"<ERROR>{str(e)}".encode())
        except Exception as e:
            print(f"[-] Failed to send error to client: {e}")
            
    finally:
        client_socket.close()
        CONNECTION_POOL.release()
        logging.info(f"[+] Closed connection with {addr}")
        print(f"[+] Closed {addr}")

def main():
    """
    Main function for the server.

    Initializes a server socket, binds it to the hostname and port, and starts
    listening for incoming connections. When a client connects, a new thread is
    created to handle the client using the handle_client function. This allows the
    server to handle multiple clients concurrently.

    The server will shut down gracefully when either SIGINT (Ctrl+C) or SIGTERM
    is received.
    """
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_socket.bind(('', CLIENT_PORT))  # Bind to all interfaces
    server_socket.listen(MAX_CONNECTIONS)

    # Graceful shutdown handling
    shutdown_flag = False
    
    def signal_handler(sig, frame):
        nonlocal shutdown_flag
        shutdown_flag = True
        server_socket.close()
        logging.info("Server shutting down")
        print("[!] Server shutting down...")

    # Register signal handlers
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)

    # Start connection discovery, Display connection details
    print(f"[+] Server running on port {CLIENT_PORT}")
    
    while not shutdown_flag:
        try:
            client_socket, addr = server_socket.accept()
            CONNECTION_POOL.acquire()
            client_thread = threading.Thread(
                target=handle_clients1,
                args=(client_socket, addr),
                daemon=True
            )
            client_thread.start()
            print(f"[+] Active connections: {threading.active_count() - 1}")
        except OSError:
            if shutdown_flag:
                break
            else:
                logging.error("Socket error occurred", exc_info=True)
                
if __name__ == "__main__": 
    main()
