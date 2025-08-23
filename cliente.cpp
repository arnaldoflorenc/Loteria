#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <thread>

using namespace std;

void inputcliente (int clientSocket){
    string msg;
    while (true){    
    getline(cin, msg);
    send (clientSocket, msg.c_str(), msg.size(), 0);
    }
}

void recebecliente (int clientSocket){
    char buffer[1024] = {0};
    while (true){
    recv(clientSocket, buffer, sizeof(buffer), 0);
    cout<<"Mensagem do servidor: "<<buffer<<endl;
    memset(buffer, 0, sizeof(buffer));
    }

}

int main(){

    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAdress;
    serverAdress.sin_family = AF_INET;
    serverAdress.sin_port = htons(8080);
    serverAdress.sin_addr.s_addr = INADDR_ANY;

    connect(clientSocket, (struct sockaddr*)&serverAdress, sizeof(serverAdress));
    
    const char* message = "Ola do cliente!";
    send(clientSocket, message, strlen(message),0);

    thread t1(inputcliente, clientSocket);
    thread t2(recebecliente, clientSocket);

    t1.join();
    t2.join();


    close(clientSocket);
    return 0;
}