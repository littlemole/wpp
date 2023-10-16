#!/bin/bash

openssl req -x509 -newkey rsa:4096 -sha256 -days 3650 -nodes \
  -keyout server.key -out server.crt -subj "/CN=localhost" \
  -addext "subjectAltName=DNS:localhost,IP:127.0.0.1"
  
cat server.key > server.pem
cat server.crt >> server.pem
