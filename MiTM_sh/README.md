Important Notes
[Better Cap Notes]
    
    HTTPS Limitations: Modern browsers enforce HSTS, making SSL stripping less effective. Use a trusted CA certificate for full decryption.
    Encrypted Protocols: SSH, RDP, and Kerberos use encryption; intercepting them requires advanced techniques (e.g., exploiting weak algorithms).
    Network Segmentation: Divide your network into smaller segments to limit lateral movement and reduce the attack surfacE
    USE Tool to generate SSL-Certs for testing purposes only

[Running the code]

    sudo bettercap -caplet fake_servers.cap

[Key Components Explained]

    ARP Spoofing: Redirects traffic through your machine.
    DNS Spoofing: Resolves specified domains to your IP (e.g., *.example.com → 192.168.1.100).
    HTTP/HTTPS Proxy: Intercepts web traffic (use sslstrip for HTTPS downgrade attacks).


Usage: 

    sudo bettercap -caplet network_monitor.cap (attack)
    wireshark -r /root/bettercap_logs.pcap (analaysis)



"""    
    [Ettercap Notes] -
        [MITM-PROXY]: Proxy Configuration: Ensure mitmproxy runs on port 8080 and your firewall allows traffic. HTTPS Limitations: This script only redirects HTTP. For HTTPS, combine with sslstrip or use a trusted CA certificate and riderct to proxy. This gives better certainty of the attack success.
"""

