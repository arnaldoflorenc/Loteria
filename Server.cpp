#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <list>
#include <string>

list<int> aposta;

using namespace std;

void escutar(){

}

void sorteio(list<int> sorteada){

}

int main(){
    list<int> sorteada;

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAdress;
    serverAdress.sin_family = AF_INET;
    serverAdress.sin_port = htons(8080);
    serverAdress.sin_addr.s_addr = INADDR_ANY;

    bind(serverSocket, (struct sockaddr*)&serverAdress, sizeof(serverAdress));
    listen(serverSocket,5);

    int clientSocket = accept(serverSocket, nullptr, nullptr);
    char buffer[1024] = {0};
    recv(clientSocket, buffer, sizeof(buffer), 0);
    cout<<"MSG cliente: "<<buffer<<endl;

    close(serverSocket);


    return 0;
}