import os
import socket as SOCKET_
import time
import numpy
import sys
import select  ## OS LEEL loop
import io
from numpy.lib.format import BUFFER_SIZE

###########################################
##  TODO: MAKE STRUCTURE FOR FILE TRANSFER AND CHAT AND CLIENT/SERVER INFO
### TODO: MAKE DATASTRUCTURES A CLASS; SEPERATE FILES BY INDIVIDUAL FUNCTIONS (CWD, CHILD, PARENT, ETC)
### TODO: ADD THREADING TO INCLUDE MULTIPLE CLIENTS AND SERVERS
## TODO: Add Chat Functionality and Simple FLASK API
## TODO: MAKE A CLASS FOR FILE TRANSFER
## TODO: ADD A CLASS FOR CHAT FUNCTIONALITY
## TODO: ADD A CLASS FOR FLASK API
###########################################

## server_ALPHA.py
## TODO: 1. Add a class for socket // 2. Add a class for file transfer // 3.  Add a class for chat // 4. Add a class for flask
## TODO: Add Conditional for IPV4 and IPV6 //
## TODO: 1. Add a class for socket // 2. Add a class for file transfer // 3.  Add a class for chat // 4. Add a class for flask


## 01_sender.py
## TODO: Add Conditional for IPV4 and IPV6 //

BUFFER_SIZE:int = 8192
CLIENT_PORT:int = 22223
CLIENT_HOSTNAME = SOCKET_.gethostname()
CLIENT_IP = SOCKET_.gethostbyname(CLIENT_HOSTNAME)
socket = SOCKET_.socket(SOCKET_.AF_INET, SOCKET_.SOCK_STREAM)
print(f"[!] connected to {CLIENT_HOSTNAME} with IP {CLIENT_IP} at port: {CLIENT_PORT}")
print(f"[!]  Initiating connection..{type(socket)} + {socket}")

## 1. Create a socket
try:
    ## TODO: Add Conditional for IPV4 and IPV6 //
    CLIENT_HOSTNAME = socket.gethostname()
    CLIENT_IP = socket.gethostbyname(CLIENT_HOSTNAME)
    socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    # socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    ## 1a. Display connection details
    print(f"[!] connected to {CLIENT_HOSTNAME} with IP {CLIENT_IP} at port: {CLIENT_PORT}")
    print(f"[!]  Initiating connection..{type(socket)} + {socket}")
    print(f"[!] {socket.getsockname()}")  ## getsockname()

except socket.error as e:
    print(f"[ERROR] Socket error: {e}")
try:
    ## 2. Bind the socket to the IP and PORT
    socket.bind((CLIENT_IP, CLIENT_PORT))
    socket.listen(5)  # sets max que  to 5
    print(f"[+] {socket.getsockname()}")

    ## accepting connection
    file_name = input("[SYSTEM] Enter File Name: ")
    file_size = os.path.getsize(file_name)

    '''sending a file from a server to a client over a socket connection. '''
    client, addr = socket.accept()
    print(f"[+] connected to {client}, {addr}")
    time.sleep(1)

    bytes_name = client.send(file_name.encode())
    time.sleep(.5)
    bytes_sent = client.send(str(file_size).encode())

    send_count = 0
    buffer = file_size
    send_start = time.time()

    ''' open and read as binary for parsing '''
    print(f"[+] {file_name} is {file_size} bytes \n[SYSTEM].. Initiating File Transfer.")
    while buffer != file_size:
        elapsed_time = time.time - file_size - send_start
        print(f"[+] {file_name} Transfer in progress: {send_count} of {file_size} bytes sent. \n[!] Elapsed Time: {elapsed_time}")

        with open(file_name, "rb") as file:
            data = file.read(8192)

            if not (data):
                print(f"[SYSTEM] {file_name} Transfer Complete, {send_count} of {file_size} bytes sent.")
                break  # <- end of F/T

            client.sendall(data)
            send_count += len(data)  ## byte format established line 63
            send_curr = time.time()

    send_end = time.time()
    total_time = send_end - send_start
    socket.close()  # stop
    print(f"[SYSTEM] {file_name} Transfer Complete in: {total_time}")


except IOError as e:
    print(f"[ERROR] File I/O error: {e}")
except Exception as e:
    print(f"[ERROR] Unexpected error: {e}")

finally:
    if 'socket' in locals():
        socket.close()
    print("[SYSTEM] Socket closed.")

    if 'client' in locals():
        client.close()
    print("[SYSTEM] Client socket closed.")
    if 'file' in locals():
        file.close()