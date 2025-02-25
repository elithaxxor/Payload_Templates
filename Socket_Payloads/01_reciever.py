import os
import socket
import time
import sys
from colorama import Fore, Style, init

# This script is a simple file transfer client that connects to a server and receives a file.
description = repr(
    "This script is a simple file transfer client that connects to a server and receives a file.\n"
    "It uses the socket library for network communication and colorama for colored terminal output.\n"
    "The script prompts the user for a server IP address and port, connects to the server, and receives a file.\n"
    "It also displays the progress of the file transfer and the time taken to complete the transfer."
    "--------------------------------------------------------------------------------------------------\n\n"
)
init(autoreset=True)
print(Fore.YELLOW + description)

print(Style.BRIGHT + Fore.CYAN + "\nFile Transfer Client")

class ClientConfig:
    def __init__(self):
        self.DEFAULT_SERVER_IP = "192.168.1.77"
        self.CPORT = 5555
        self.server_host = self.get_server_ip()
        self.client_name = socket.gethostname()
        self.client_IP = socket.gethostbyname(self.client_name)
        self.client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    def get_server_ip(self):
        user_choice = input(Fore.YELLOW + '[!]** Press 1 for default box (192.168.1.77) or any other key for custom IP: ')
        if user_choice == '1':
            return self.DEFAULT_SERVER_IP
        else:
            return input(Fore.YELLOW + '[!]** Enter the server IP for connection: ')

def main():
    config = ClientConfig()

    try:
        # Attempt to connect to the server
        config.client_socket.connect((config.server_host, config.CPORT))
        print(Fore.GREEN + f'[!] - [CONNECTING..] {config.server_host} : {config.CPORT}')
        print(Fore.GREEN + f'Hello {config.client_name}, you are connected on: {config.client_IP}')

        # Display server's local IP
        server_IP = config.client_socket.getsockname()[0]
        print(Fore.CYAN + f'[+] Server Local IP: {server_IP}')

    except Exception as e:
        print(Fore.RED + f'[-]*[ERROR] Unable to connect to: {config.server_host} - {str(e)}')
        print("restarting main function")  # Exit if unable to connect
        main()

    # Receive file parameters
    file_name = config.client_socket.recv(100).decode('utf-8')
    file_size = config.client_socket.recv(100).decode('utf-8') # Convert to int
    print(Fore.GREEN + f'[SYSTEM]** {file_name} is : {file_size} bytes \n[SYSTEM].. Initiating File Transfer.')

    # Open and write the file into BINARY
    start_time = time.time()
    file_size_int = int(file_size)
    with open(file_name, 'wb') as file:
        receive_count = 0
        while receive_count < file_size_int:
            data = config.client_socket.recv(8192)
            if not data:
                break
            file.write(data)
            receive_count += len(data)

            # Display progress
            progress = (receive_count / file_size) * 100
            print(Fore.MAGENTA + f'\r[SYSTEM] Progress: {progress:.2f}% ({receive_count}/{file_size} bytes)', end='')

    end_time = time.time()

    # Calculate time to complete
    time_to_complete = end_time - start_time
    print(Fore.GREEN + f'\n[SYSTEM] File Transfer Complete in {time_to_complete:.2f} seconds')
    # Close the socket
    config.client_socket.close()

if __name__ == "__main__":
    main()