#!/bin/bash
# Compila o servidor e o cliente

set -e

# Caminhos
SERVER_SRC="Server.cpp"
CLIENT_SRC="cliente.cpp"
SERVER_BIN="output/Server"
CLIENT_BIN="output/cliente"

# Cria diretório de saída se não existir
mkdir -p output

echo "Compilando servidor..."
g++ -std=c++11 -pthread "$SERVER_SRC" -o "$SERVER_BIN"
echo "Compilando cliente..."
g++ -std=c++11 -pthread "$CLIENT_SRC" -o "$CLIENT_BIN"

echo "Para rodar o servidor, use:"
echo "  ./output/Server"
echo "Para rodar o cliente, use:"
echo "  ./output/cliente"
