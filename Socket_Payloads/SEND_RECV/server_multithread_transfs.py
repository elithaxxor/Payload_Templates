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
connection_pool = threading.BoundedSemaphore(MAX_CONNECTIONS)

def send_directory_listing(client_socket):
    """Send list of available files to client"""
    try:
        # Get list of files (excluding directories)
        files = [f for f in os.listdir('.') if os.path.isfile(f)]
        print("[+] Files available \n", files)
        # Create formatted listing
        listing = "\n".join([
            f"{file} - {os.path.getsize(file)} bytes" 
            for file in files
        ]) or "No files available"
        
        print(f"[+] {client_socket} requested Listings available  \n", listing)
        logging.info(f"[+] {client_socket} requested Listings available  \n", listing)

        # Send listing with header
        header = struct.pack('Q', len(listing))
        client_socket.sendall(header)
        client_socket.sendall(listing.encode())
        print("[+] Sent directory to client: ", client_socket)
        logging.info("Sent directory listing to client")
        
    except Exception as e:
        logging.error(f"Directory listing error: {str(e)}")
        print("[-] Directory listing occurred, sending blank struct.. : error: \n", e)
        client_socket.sendall(struct.pack('Q', 0))  # Send empty listing
        
        
def handle_client(client_socket, addr):
    try:
        '''#1 ------------ Connection CENTER --------------'''

        client_socket.settimeout(TIMEOUT)
        logging.info(f"Connection from {addr}")
        print(f"[+] Connected to {addr}")
        
'''#2 ------------ COMMAND CENTER --------------'''


## 4BYTE Commands from client to server
        command = client_socket.recv(4).decode() 
        print(f"[!] Command {Command} called by: {addr}")
        logging.info(f"[!] Command {Command} called by: {addr}")
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
            pass
        
        
        elif command == 'FILE':
            header = client_socket.recv(58)
            print("Header: \r", header)
            
            if len(header) != 58:
                raise ValueError(f"Invalid header length: {len(header)}")

            file_name, file_size = struct.unpack('50sQ', header) # 58 bytes 50 for file name 8 for size
            file_name = file_name.decode('utf-8').rstrip('\x00')
            print(file_name)
            file_name = os.path.basename(file_name)  # Security sanitizatio
            print(file_name)
            file_name = os.path.join(os.getcwd(), file_name)  # Full path for security
            print(file_name)

            
            
            if not os.path.exists(file_name):
                raise FileNotFoundError(f"File {file_name} not found")

            logging.info(f"Sending {file_name} ({file_size} bytes)")
            print(f"[*] Transferring {file_name} ({file_size} bytes)")

            bytes_sent = 0
            start_time = time.time()

            with open(file_name, "rb") as f:
                while True:
                    data = f.read(BUFFER_SIZE)
                    if not data:
                        break
                    client_socket.sendall(data)
                    bytes_sent += len(data)

                    # Progress logging
                    if bytes_sent % (BUFFER_SIZE * 10) == 0:  # Log every 10 buffers
                        print(f"Sent {bytes_sent}/{file_size} bytes")

            duration = time.time() - start_time
            logging.info(f"Transfer complete in {duration:.2f}s")
            print(f"[+] Transfer completed in {duration:.2f} seconds")

        except Exception as e:
            logging.error(f"Error with {addr}: {str(e)}")
            print(f"[-] Error: {str(e)}")
            try:
                client_socket.send(f"<ERROR>{str(e)}".encode())
            except:
                pass

    finally:
        client_socket.close()
        connection_pool.release()
        logging.info(f"Closed connection with {addr}")
        print(f"[-] Closed {addr}")

def main():

    // #1. Initialize server socket
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

    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)

    // #1. Start connection discovery, Display connection details
    print(f"[+] Server running on port {CLIENT_PORT}")
    while not shutdown_flag:
        try:
            client_socket, addr = server_socket.accept()
            with connection_pool:
                client_thread = threading.Thread(
                    target=handle_client,
                    args=(client_socket, addr),
                    daemon=True
                )
                client_thread.start()
                print(f"[+] Active connections: {threading.active_count() - 1}")

        except OSError:
            if shutdown_flag:
                break

if __name__ == "__main__":
    main()
