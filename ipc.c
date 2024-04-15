/*
Uma Indústria de Alimentos de Santa Catarina chamada FoodSec S.A. possui a tarefa de escanear 
alimentos por meio de câmeras e verificar se os mesmos estão corretos. Os alimentos podem passar 
por uma das duas esteiras disponíveis. As duas esteiras são controladas por um por um único 
computador centralizado. Esse computador recebe dados de um sensor em cada uma das esteiras que 
captura a contagem de itens que são identificados como seguros. A contagem é exibida em um display 
perto das esteiras (todos os displays são controlados pela mesma função, é apenas uma replicação)

A empresa está avaliando a viabilidade de implementação nos Sistemas Operacionais Windows e Linux 
(qualquer distro) e também em duas formas de IPC. Com isso, você deve implementar duas versões 
da solução nos dois sistemas operacionais e comparar qual é a que apresenta o melhor desempenho
quanto ao tempo de computação (tempo de processamento). A primeira solução deve usar IPC do tipo 
pipe (de preferência nomeado), já a segunda solução deve ser implementada via multithread (ou multithread, como queira descrever). Logo, deve ser implementado as duas soluções para Windows e as
duas para Linux. Você poderá usar o Codespace ou Cocalc, por exemplo, para implementar a versão 
em Linux. Para Windows, você pode usar máquinas virtuais também.

Os códigos disponibilizados pelo professor (de teste e exercícios) podem ser usados para o 
desenvolvimento do trabalho (na verdade, é extremamente recomendado). A avaliação do tempo deve 
ser entre a execução da contagem e na comunicação, sendo recomendado desconsiderar a criação de 
threads, pipe ou memória compartilhada. 

() revisar se atende ao que foi pedido
() revisar formatacao
() revisar funcoes
() revisar versao linux 
() revisar versao windows
() fazer struct esteira
() criar um metodo na struct esteira (que possui um intervalo) para atualizar o sensor em determinado intervalo

*/

// implementação ipc em linux
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define BUFFER_SIZE 100

void atualiza_sensor_esteira1(int pipe_fd);
void atualiza_sensor_esteira2(int pipe_fd);
void atualiza_display(int pipe_fd);

int main() {
    int pipe_fd;
    char *pipe_name = "/tmp/foodsecpipe";

    // Criar o pipe nomeado
    mkfifo(pipe_name, 0666);

    // Abrir o pipe para leitura e escrita
    pipe_fd = open(pipe_name, O_RDWR);

    // Verificar se a abertura do pipe foi bem-sucedida
    if (pipe_fd == -1) {
        perror("Erro ao abrir o pipe nomeado");
        return 1;
    }

    // Criar as threads para atualizar as esteiras e o display
    atualiza_sensor_esteira1(pipe_fd);
    atualiza_sensor_esteira2(pipe_fd);
    atualiza_display(pipe_fd);

    // Fechar o pipe
    close(pipe_fd);

    // Remover o pipe nomeado
    unlink(pipe_name);

    return 0;
}

void atualiza_sensor_esteira1(int pipe_fd) {
    int lido = 0;
    int intervalo = 2000000; // 2 segundos
    char buffer[BUFFER_SIZE];

    while (1) {
        // Simular a leitura do sensor e envio dos dados para o pipe
        sprintf(buffer, "Esteira 1 - Itens: %d, Peso Total: %d Kg", lido, lido * 5);
        write(pipe_fd, buffer, BUFFER_SIZE);

        usleep(intervalo); // Esperar 2 segundos
        lido++;
    }
}

void atualiza_sensor_esteira2(int pipe_fd) {
    int lido = 0;
    int intervalo = 1000000; // 1 segundo
    char buffer[BUFFER_SIZE];

    while (1) {
        // Simular a leitura do sensor e envio dos dados para o pipe
        sprintf(buffer, "Esteira 2 - Itens: %d, Peso Total: %d Kg", lido, lido * 2);
        write(pipe_fd, buffer, BUFFER_SIZE);

        usleep(intervalo); // Esperar 1 segundo
        lido++;
    }
}

void atualiza_display(int pipe_fd) {
    char buffer[BUFFER_SIZE];

    while (1) {
        // Ler os dados do pipe e exibir no display
        read(pipe_fd, buffer, BUFFER_SIZE);
        printf("%s\n", buffer);
    }
}
