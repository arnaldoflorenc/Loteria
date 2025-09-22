#include <atomic>
#include <mutex>
#include <condition_variable>
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

std::atomic<int> clientes_ativos(0);
int LIMITE_CLIENTES = 1;
std::mutex mtx_limite;

bool registraAposta(string& msg, list<int>& aposta_local, ConfigLoteria& config){
    stringstream msg_client(msg);
    string token;
    while(msg_client >> token){
        for (char c : token){
            if(!isdigit(c)){
                cout << "Aposta inválida! Apenas números são permitidos." << endl;
                return false;
            }
        }
        int token_aux = stoi(token);
        if(token_aux < config.inicio || token_aux > config.fim){
            cout << "Aposta inválida! Números devem estar entre "
                 << config.inicio << " e " << config.fim << endl;
            return false;
        }
        aposta_local.push_back(token_aux);
    }
    cout << "Aposta registrada! -> \t" << msg << endl;
    return true;
}

void tratarComando(const string& msg, ConfigLoteria& config) {
    stringstream ss(msg);
    string comando;
    int valor;
    if(msg.find(":inicio") == 0) {
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

void escutar(int clientSocket, list<int>& aposta_local, ConfigLoteria& config,
             mutex& mtx_aposta, condition_variable& cv_aposta, bool& aposta_completa) 
{
    char buffer[1024];

    while (true) {
        ssize_t tam = read(clientSocket, buffer, sizeof(buffer) - 1);
        if (tam <= 0) {
            cout << "Cliente desconectado." << endl;
            close(clientSocket);
            return; // sai só se o cliente fechou a conexão
        }

        buffer[tam] = '\0';
        string msg(buffer);
        //cout << "[DEBUG] Recebido do cliente: '" << msg << "'" << endl;

        // Se for comando
        if (!msg.empty() && msg[0] == ':') {
            if (msg.find(":exit") == 0) {
                cout << "Cliente solicitou saída. Encerrando conexão." << endl;
                clientes_ativos--;
                close(clientSocket);
                return;
            }
            tratarComando(msg, config);
            continue;
        }

        // Se for aposta
        {
            lock_guard<mutex> lock(mtx_aposta);
            if (registraAposta(msg, aposta_local, config)) {
                cout << "Aposta computada!!" << endl;

                if ((int)aposta_local.size() >= config.qtd) {
                    aposta_completa = true;
                    cv_aposta.notify_one();
                }
            }
        }
    }
}

void sorteio(int clientSocket, list<int>& aposta_local, ConfigLoteria& config,
             mutex& mtx_aposta, condition_variable& cv_aposta, bool& aposta_completa) 
{
    while (true) {
        unique_lock<mutex> lock(mtx_aposta);

        // espera até que uma aposta esteja completa
        cv_aposta.wait(lock, [&] { return aposta_completa; });

        // já tenho aposta suficiente, copia para trabalhar
        list<int> aposta_copia = aposta_local;

        // resetar para permitir nova aposta
        aposta_local.clear();
        aposta_completa = false;
        lock.unlock();

        // espera um tempo antes de sortear (simulação)
        this_thread::sleep_for(chrono::seconds(5));

        // gera números sorteados
        list<int> num_sorteados;
        for (int i = 0; i < config.qtd; i++) {
            num_sorteados.push_back(config.inicio + rand() % (config.fim - config.inicio + 1));
        }

        // compara aposta
        string result = (aposta_copia == num_sorteados) 
                        ? "Você ganhou!" 
                        : "Você perdeu... tente novamente.";

        stringstream aux;
        for (int n : num_sorteados) {
            aux << n << " ";
        }
        result += "\nNúmeros sorteados: " + aux.str();

        ssize_t enviado = send(clientSocket, result.c_str(), result.size(), 0);
        if (enviado < 0) {
            cerr << "Erro ao enviar resultado ao cliente." << endl;
            return; // encerra se não conseguir enviar
        } else {
            cout << "Resultado enviado ao cliente!" << endl;
        }
    }
}

void handle_client(int clientSocket) {
    ConfigLoteria config;
    list<int> aposta_local;
    mutex mtx_aposta;
    condition_variable cv_aposta;
    bool aposta_completa = false;

    thread t1([&] {
        escutar(clientSocket, aposta_local, config,
                mtx_aposta, cv_aposta, aposta_completa);
    });

    thread t2([&] {
        sorteio(clientSocket, aposta_local, config,
                mtx_aposta, cv_aposta, aposta_completa);
    });

    t1.join();
    t2.join();

    cout << "Cliente desconectado, liberando recursos.\n";
}

int main(int argc, char* argv[]){ 
    // hora em sec para seed
    time_t now = time(nullptr);
    tm* hr_local = localtime(&now);
    int seg_seed = (hr_local->tm_hour * 3600) + (hr_local->tm_min * 60);
    srand(seg_seed);

    // Lê limite de clientes do argumento de linha de comando
    if (argc > 1) {
        LIMITE_CLIENTES = atoi(argv[1]);
        if (LIMITE_CLIENTES < 1) LIMITE_CLIENTES = 1;
    }

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        cerr << "Erro ao criar socket" << endl;
        return -1;
    }
    sockaddr_in serverAdress;
    serverAdress.sin_family = AF_INET;
    serverAdress.sin_port = htons(8080);
    serverAdress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAdress, sizeof(serverAdress)) < 0) {
        cerr << "Erro ao fazer bind" << endl;
        close(serverSocket);
        return -1;
    }
    if (listen(serverSocket, 5) < 0) {
        cerr << "Erro ao escutar" << endl;
        close(serverSocket);
        return -1;
    }

    cout << "Servidor pronto para receber conexões\n";
    cout << "Limite de clientes: " << LIMITE_CLIENTES << endl;

    while (true) {
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket < 0) {
            cerr << "Erro ao aceitar conexão." << endl;
            continue;
        }

        lock_guard<std::mutex> lock(mtx_limite);
        if (clientes_ativos.load() >= LIMITE_CLIENTES) {
            string msg = "SERVER_FULL";
            send(clientSocket, msg.c_str(), msg.size(), 0);
            close(clientSocket);
            cout << "Conexão recusada: limite de clientes atingido." << endl;
            continue;
        }

        clientes_ativos++;
        cout << "Cliente conectado.\n";

        thread t([clientSocket]() {
            handle_client(clientSocket);
            clientes_ativos--;
        });
        t.detach();
    }

    close(serverSocket);
    return 0;
}
