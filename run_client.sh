#!/bin/bash/

host=$(./cliente.out)
echo "La dirección IP es: $host"

python3 ./client.py -s "$host" -p 8888