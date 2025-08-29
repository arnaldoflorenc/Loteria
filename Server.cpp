#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sstream>
#include <unistd.h>
#include <list>
#include <thread>

using namespace std;

list<int> aposta;

void registraAposta(string& msg){
    stringstream msg_client(msg);
    int valor;
    while(msg_client >> valor){
        aposta.push_back(valor);
    }

}

void escutar(int clientSocket){
    char buffer[1024];
    while(true){
        ssize_t tam = read(clientSocket, buffer, sizeof(buffer)-1);
        if(tam <= 0){
            close(clientSocket);
            return;
        }
        buffer[tam] = '\0';
        string msg(buffer);

        registraAposta(msg);

        cout << "Aposta Recebida!"<<endl;
        
        for(int ap : aposta){
            cout <<"Aposta computada!!"<<ap<<endl;
        }
        
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