#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <list>
#include <thread>

list<int> aposta;

using namespace std;

void escutar(int clientSocket){
    int apost_client;
    while(true){
        ssize_t tam = read(clientSocket, &apost_client, sizeof(apost_client));
        if(tam <= 0){
            close(clientSocket);
            return;
        }
        aposta.push_back(apost_client);
        cout <<"Aposta computada!!"<<aposta<<endl;
    }
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

    while(true){
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        thread(escutar,clientSocket).detach();
    }

    close(serverSocket);


    return 0;
}