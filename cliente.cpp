#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace std;

int main(){

    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAdress;
    serverAdress.sin_family = AF_INET;
    serverAdress.sin_port = htons(8080);
    serverAdress.sin_addr.s_addr = INADDR_ANY;

    connect(clientSocket, (struct sockaddr*)&serverAdress, sizeof(serverAdress));
    
    const char* message = "Ola do cliente!";
    send(clientSocket, message, strlen(message),0);

    close(clientSocket);
    return 0;
}