#!/bin/bash
# start_ftp_server.sh
# This script installs pyftpdlib (if necessary) and starts an FTP server on port 21 with write access.

# Function to check if a Python module is installed.
is_module_installed() {
    python -c "import $1" 2>/dev/null
}

# Check for pyftpdlib and install it if not present.
if ! is_module_installed pyftpdlib; then
    echo "[*] pyftpdlib not found. Installing..."
    pip install pyftpdlib || { echo "[-] Error installing pyftpdlib. Exiting."; exit 1; }
else
    echo "[*] pyftpdlib is already installed."
fi

echo "[*] Starting FTP server on port 21 with write permissions..."
# Starting the FTP server. The '-w' flag enables write access.
sudo python -m pyftpdlib -p 21 -w