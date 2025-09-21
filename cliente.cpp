#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <atomic>

using namespace std;

atomic<bool> encerrar(false); 

void inputcliente (int clientSocket){
    string msg;
    //cout << "Digite sua aposta :" << endl;
    while (!encerrar) {
        fd_set set;
        FD_ZERO(&set);
        FD_SET(STDIN_FILENO, &set);

        struct timeval timeout;
        timeout.tv_sec = 1; 
        timeout.tv_usec = 0;

        int ready = select(STDIN_FILENO + 1, &set, NULL, NULL, &timeout);
        if (ready > 0 && FD_ISSET(STDIN_FILENO, &set)) {
            getline(cin, msg);
            if (!msg.empty()) {
                ssize_t sent = send(clientSocket, msg.c_str(), msg.size(), 0);
                if (sent < 0) {
                    cerr << "Erro ao enviar mensagem ao servidor." << endl;
                    encerrar = true;
                    fclose(stdin);
                    close(clientSocket);
                    break;
                }
            }
            if (msg == ":exit") {
                encerrar = true;
                close(clientSocket);
                break;
            }
        }
        // Se encerrar for true, o loop termina sem bloquear
    }
}

void recebecliente (int clientSocket){
    char buffer[1024] = {0};
        while (!encerrar){
            int n = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (n <= 0) {
                cerr << "Conexão encerrada pelo servidor ou erro na recepção." << endl;
                encerrar = true;
                fclose(stdin);
                close(clientSocket);
                break;
            }
            string resposta(buffer);
            if(resposta == "SERVER_FULL"){
                cout << "Conexão recusada!! Server lotado! FAZ O L"<<endl;
                encerrar = true;
                fclose(stdin);
                close(clientSocket);
                break;
            }
            if(encerrar){
                fclose(stdin);
                close(clientSocket);
                break;
            }
            cout << "Mensagem do servidor: " << resposta << endl;
            memset(buffer, 0, sizeof(buffer));
    }
}

int main(){
    auto now = chrono::system_clock::now();
    time_t tt = chrono::system_clock::to_time_t(now);
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        cerr << "Erro ao criar socket" << endl;
        return -1;
    }
    tm* now_tm = localtime(&tt);
    string msg;

    sockaddr_in serverAdress;
    serverAdress.sin_family = AF_INET;
    serverAdress.sin_port = htons(8080);
    serverAdress.sin_addr.s_addr = INADDR_ANY;



    if (connect(clientSocket, (struct sockaddr*)&serverAdress, sizeof(serverAdress)) < 0) {
        cerr << "Erro ao conectar ao servidor" << endl;
        close(clientSocket);
        return -1;
    }

    cout<< put_time(now_tm, "%H:%M:%S") << ": CONECTADO!!"<< endl;

    // Prompt de comandos
    cout << "\nComandos disponíveis:\n";
    cout << ":inicio <NUM>   - Define início do intervalo da loteria\n";
    cout << ":fim <NUM>      - Define fim do intervalo da loteria\n";
    cout << ":qtd <NUM>      - Define quantidade de números sorteados\n";
    cout << ":exit           - Sair do sistema\n";
    cout << "Aposta:         - Digite os números separados por espaço\n";
    cout << "---------------------------------------------\n";

   
    thread t1(inputcliente, clientSocket);
    thread t2(recebecliente, clientSocket);
    t1.join();
    t2.join();
    close(clientSocket);
    return 0;
}