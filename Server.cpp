#include <atomic>
#include <mutex>
std::atomic<int> clientes_ativos(0);
int LIMITE_CLIENTES = 1;
std::mutex mtx_limite;
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sstream>
#include <unistd.h>
#include <ctime>
#include <chrono>
#include <list>
#include <thread>
#include <cstdlib>

using namespace std;

struct ConfigLoteria {
    int inicio = 0;
    int fim = 100;
    int qtd = 5;
};

void registraAposta(string& msg, list<int>& aposta_local, ConfigLoteria& config){
    stringstream msg_client(msg);
    string token;
    while(msg_client >> token){
        for (char c : token){
            if(!isdigit(c)){
                cout << "Aposta inválida! Apenas números são permitidos." << endl;
                return;
            }
        }
        int token_aux = stoi(token);
        if(token_aux < config.inicio || token_aux > config.fim){
            cout << "Aposta inválida! Números devem estar entre 0 e 100." << endl;
            return;
        }
        aposta_local.push_back(token_aux);
    }
    cout << "Aposta registrada! -> \t" << msg << endl;
}

void tratarComando(const string& msg, ConfigLoteria& config) {
    stringstream ss(msg);
    string comando;
    int valor;
    if(msg.find(":inicio") == 0)
    {
        ss >> comando >> valor;
        config.inicio = valor;
        cout << "Configuração alterada: inicio = " << config.inicio << endl;
    } else if (msg.find(":fim") == 0){
        ss >> comando >> valor;
        config.fim = valor;
        cout << "Configuração alterada: fim = " << config.fim << endl;
    } else if (msg.find(":qtd") == 0){
        ss >> comando >> valor;
        config.qtd = valor;
        cout << "Configuração alterada: qtd = " << config.qtd << endl;
    } else {
        cout << "Comando inválido." << endl;
    }
}

void escutar(int clientSocket, list<int>& aposta_local, ConfigLoteria& config){
    char buffer[1024];
    while(true){
        ssize_t tam = read(clientSocket, buffer, sizeof(buffer)-1);
        if(tam <= 0){
            close(clientSocket);
            return;
        }
        buffer[tam] = '\0';
        string msg(buffer);
        cout << "[DEBUG] Recebido do cliente: '" << msg << "'" << endl;

        if (msg[0] == ':')
        {
            if (msg.find(":exit") == 0) {
                cout << "Cliente solicitou saída. Encerrando conexão." << endl;
                clientes_ativos--;
                close(clientSocket);
                return; // Apenas encerra a thread do cliente
            }
            tratarComando(msg, config);
            continue;
        }

        registraAposta(msg, aposta_local, config);

        cout << "Aposta Recebida!"<<endl;
        cout <<"Aposta computada!!\n"<<endl;
        if (buffer[tam] == '\0') {
            return;
        }
    }
}

void sorteio(int clientSocket, list<int>& aposta_local, ConfigLoteria& config){
    while ((int)aposta_local.size() < config.qtd) {
        this_thread::sleep_for(chrono::milliseconds(100));
    }
    this_thread::sleep_for(chrono::seconds(5));
    list<int> num_sorteados;
    for(int i=0; i<config.qtd; i++){
        num_sorteados.push_back(config.inicio + rand() % (config.fim - config.inicio + 1));
    }
    string result = (aposta_local == num_sorteados) ? "Você ganhou" : "Você perdeu... tente novamente";
    stringstream aux;
    for(int n : num_sorteados){
        aux << n <<" ";
    }
    result += "\nNúmeros Sorteados: " + aux.str();
    send(clientSocket, result.c_str(), result.size(), 0);
    cout << "Resultado enviado ao cliente!" << endl;
}

void handle_client(int clientSocket){
    ConfigLoteria config;
    list<int> aposta_local;
    while(true){
        thread t1(escutar, clientSocket, std::ref(aposta_local), std::ref(config));
        thread t2(sorteio, clientSocket, std::ref(aposta_local), std::ref(config));
        t1.join();
        t2.join();
        aposta_local.clear();
        cout<<"Aposta finalizada, esperando nova aposta do mesmo cliente\n";
    }
}



int main(int argc, char* argv[]){ 
    //hora em sec para seed
    time_t now = time(nullptr);
    tm* hr_local = localtime(&now);
    int seg_seed = (hr_local->tm_hour * 3600) + (hr_local->tm_min * 60);
    srand(seg_seed);

    // Lê limite de clientes do argumento de linha de comando
    if (argc > 1) {
        LIMITE_CLIENTES = atoi(argv[1]);
        if (LIMITE_CLIENTES < 1) LIMITE_CLIENTES = 1;
    }

    //inicio conectividade do server
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAdress;
    serverAdress.sin_family = AF_INET;
    serverAdress.sin_port = htons(8080);
    serverAdress.sin_addr.s_addr = INADDR_ANY;
    bind(serverSocket, (struct sockaddr*)&serverAdress, sizeof(serverAdress));
    listen(serverSocket,5);
    cout<<"Servidor pronto para receber conexões\n";
    cout<<"Limite de clientes: "<<LIMITE_CLIENTES<<endl;
    while(true){
        cout << "[DEBUG] Esperando conexão..." << endl;
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        cout << "[DEBUG] accept retornou, socket: " << clientSocket << endl;
        std::lock_guard<std::mutex> lock(mtx_limite);
        if(clientes_ativos.load() >= LIMITE_CLIENTES){
            string msg = "SERVER_FULL";
            send(clientSocket, msg.c_str(), msg.size(), 0);
            close(clientSocket);
            cout << "Conexão recusada: limite de clientes atingido." << endl;
            continue;
        }
        clientes_ativos++;
        cout<<"Client conectado\n";
        if(clientSocket >= 0){
            thread t([clientSocket]() {
                handle_client(clientSocket);
                clientes_ativos--;
            });
            t.detach();
        } else {
            cerr<<"Erro ao aceitar conexão\n";
        }
    }
    cout << "Servidor encerrando..." << endl;
    close(serverSocket);
    return 0;
}