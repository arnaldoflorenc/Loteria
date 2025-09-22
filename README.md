# Loteria Cliente-Servidor em C++

Sistema de loteria em C++ usando sockets TCP. Permite múltiplos clientes, envio de apostas, configuração de intervalo e quantidade de números sorteados.

---

## Estrutura do projeto
```
.
├── server.cpp
├── client.cpp
├── build.sh # script de compilação
├── output/ # executáveis gerados
│ ├── server
│ └── client
└── README.md
```

---

## Como compilar

Execute o script de build na raiz do projeto:

```bash
./build.sh
```

---
## Como executar

#Servidor
```bash
./output/Server [limite_clientes]
```
 - Limite_clientes (opcional) - maximo de clientes simultâneos (Padrao: 1)
 - Porta padrao: 8080

#Cliente
```bash
./output/cliente
```
Se o servidor estiver cheio, a conexão é recusada

#Comandos do cliente
 - :inicio <NUM> — define início do intervalo da loteria

 - :fim <NUM> — define fim do intervalo da loteria

 - :qtd <NUM> — define quantidade de números sorteados

 - :exit — encerra a conexão

 - Apostas: digite números separados por espaço, ex.: 5 23 42 7 10
