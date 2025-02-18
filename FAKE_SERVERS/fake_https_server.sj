#!/bin/bash


# HTTP
python3 -m http.server 80

# HTTPS (generate a self-signed cert first)
openssl req -x509 -newkey rsa:4096 -nodes -out cert.pem -keyout key.pem -days 365
python3 -m http.server 443 --ssl-cert cert.pem --ssl-key key.pem

#smtp
sudo python3 -m smtpd -n -c DebuggingServer 0.0.0.0:25

#ftp
pip install pyftpdlib
python -m pyftpdlib -p 21 -w  # Anonymous write access

#dns 
sudo dnsmasq --interface=eth0 --dhcp-range=192.168.1.100,192.168.1.200,12h     