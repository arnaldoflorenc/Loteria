#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <ctime>
#include <iomanip>

using namespace std;

void inputcliente (int clientSocket){
    string msg;
    cout << "Digite sua aposta (6 números separados por espaço):" << endl;
    while (true){    
    getline(cin, msg);
    if (msg == "exit") break;
    if (!msg.empty()){
    send (clientSocket, msg.c_str(), msg.size(), 0);
    }
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
    auto now = chrono::system_clock::now();
    time_t tt = chrono::system_clock::to_time_t(now);
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    tm* now_tm = localtime(&tt);
    string msg;

    sockaddr_in serverAdress;
    serverAdress.sin_family = AF_INET;
    serverAdress.sin_port = htons(8080);
    serverAdress.sin_addr.s_addr = INADDR_ANY;

    connect(clientSocket, (struct sockaddr*)&serverAdress, sizeof(serverAdress));
    cout<< put_time(now_tm, "%H:%M:%S") << ": CONECTADO!!"<< endl;
    while(true){
    thread t1(inputcliente, clientSocket);
    thread t2(recebecliente, clientSocket);
    t1.join();
    t2.join();
    }

    close(clientSocket);
    return 0;
}