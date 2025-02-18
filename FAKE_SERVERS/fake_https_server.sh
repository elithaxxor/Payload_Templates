# HTTP
python3 -m http.server 80

# HTTPS (generate a self-signed cert first)
openssl req -x509 -newkey rsa:4096 -nodes -out cert.pem -keyout key.pem -days 365
python3 -m http.server 443 --ssl-cert cert.pem --ssl-key key.pem