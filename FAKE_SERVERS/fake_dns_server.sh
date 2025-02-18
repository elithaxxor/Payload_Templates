#!/bin/bash

sudo dnsmasq --interface=eth0 --dhcp-range=192.168.1.100,192.168.1.200,12h     



#!/bin/bash
# fake_dns.sh
# This script sets up a fake DNS server that always returns a fixed IP address for all queries.
# It uses Python with the dnslib library.

# Set the fake IP address and default port.
FAKE_IP="192.168.1.100"
DNS_PORT=53

# Check if dnslib is installed; if not, install it.
if ! python3 -c "import dnslib" &>/dev/null; then
    echo "[*] dnslib not found. Installing dnslib via pip3..."
    pip3 install dnslib || { echo "[-] Failed to install dnslib. Exiting."; exit 1; }
fi

# Create the Python script that implements the fake DNS server.
cat << 'EOF' > fake_dns.py
#!/usr/bin/env python3
import socketserver
import sys
from dnslib import DNSRecord, QTYPE, RR, A

# The fake IP address to return for all DNS queries.
FAKE_IP = "192.168.1.100"

class DNSHandler(socketserver.BaseRequestHandler):
    def handle(self):
        data, sock = self.request
        try:
            request = DNSRecord.parse(data)
        except Exception as e:
            return
        qname = request.q.qname
        qtype = QTYPE[request.q.qtype]
        print(f"Received query for: {qname} ({qtype})")
        # Build a DNS response with an A record using the fake IP.
        reply = DNSRecord(DNSRecord.header(request), q=request.q)
        reply.add_answer(RR(qname, QTYPE.A, rdata=A(FAKE_IP), ttl=60))
        sock.sendto(reply.pack(), self.client_address)

if __name__ == "__main__":
    # Allow port override via command-line argument.
    port = 53
    if len(sys.argv) > 1:
        port = int(sys.argv[1])
    print(f"Starting fake DNS server on UDP port {port} returning {FAKE_IP}")
    server = socketserver.UDPServer(('', port), DNSHandler)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("Shutting down fake DNS server.")
EOF

# Make the Python script executable.
chmod +x fake_dns.py

echo "[*] Fake DNS server script created."

# Inform the user and start the DNS server.
echo "[*] Starting fake DNS server on UDP port ${DNS_PORT} returning IP ${FAKE_IP}..."
# Binding to port 53 (a privileged port) requires sudo.
sudo ./fake_dns.py ${DNS_PORT}