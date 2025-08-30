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

enum class tipo_aposta{ //Enum que fala qual o tipo de aposta, se n houver especificando inicio e fim é padrão
    PADRAO,
    SELECIONADA
};

list<int> aposta;

tipo_aposta defTipo(){
    if(aposta.size() == 6){
        return tipo_aposta::PADRAO;
    } else{
        return tipo_aposta::SELECIONADA;
    }
}

void registraAposta(string& msg){
    stringstream msg_client(msg);    
    int valor;
    while(msg_client >> valor){
        aposta.push_back(valor);
    }
    cout << "Aposta registrada! -> \t" + msg <<endl;
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
        cout <<"Aposta computada!!\n"<<endl;
        if (buffer[tam] == '\0') {
            return;
        }
    }
}

void sorteio(int clientSocket){

    while (aposta.size() < 6) {
        this_thread::sleep_for(chrono::milliseconds(100));
    }

    this_thread::sleep_for(chrono::seconds(5));
    
    list<int> num_sorteados;
    if(defTipo() == tipo_aposta::PADRAO){
        for(int i=0; i<6; i++){//caso padrão sorteia de 0 a 60
            num_sorteados.push_back(rand()%100);    
        }
    }else{
        int minimo = aposta.front();
        int max = aposta.front();
        aposta.pop_front();
        aposta.pop_front();
        
        for(int i=0; i<6; i++){//caso selecionado sorteia de minimo até max
            num_sorteados.push_back(minimo + rand()%max);    
        }
    }
    string result = (aposta == num_sorteados) ? "Você ganhou!!" : "Você perdeu... tente novamente";
    stringstream aux;
    for(int n : num_sorteados){
        aux << n <<" ";
    }
    result += "\nNúmeros Sorteados: " + aux.str();
    send(clientSocket, result.c_str(), result.size(), 0);
    cout << "Resultado enviado ao cliente!" << endl;
}

int main(){ 
    //hora em sec para seed
    time_t now = time(nullptr);
    tm* hr_local = localtime(&now);
    int seg_seed = (hr_local->tm_hour * 3600) + (hr_local->tm_min * 60);
    srand(seg_seed);

    //inicio conectividade do server
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAdress;
    serverAdress.sin_family = AF_INET;
    serverAdress.sin_port = htons(8080);
    serverAdress.sin_addr.s_addr = INADDR_ANY;
    bind(serverSocket, (struct sockaddr*)&serverAdress, sizeof(serverAdress));
    listen(serverSocket,5);
    cout<<"Servidor pronto para receber conexões\n";
    int clientSocket = accept(serverSocket, nullptr, nullptr);
    cout<<"Client conectado\n";
    while(true){
        

        thread t1(escutar,clientSocket);//escutar novos clientes
        thread t2(sorteio, clientSocket);//sortear nums
        t1.join();
        t2.join();

        aposta.clear();
        cout<<"Aposta finalizada, esperando novo cliente\n";
    }
    
    cout << "Servidor encerrando..." << endl;


    close(serverSocket);


    return 0;
}