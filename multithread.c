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

// implementação multithread em linux
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

pthread_mutex_t exclusao_mutua = PTHREAD_MUTEX_INITIALIZER; //LOCK da seção crítica

double sensor_lido = 0;
int itens_lidos = 0;

void atualiza_sensor_esteira1();
void atualiza_sensor_esteira2();
void atualiza_display();

struct Esteira{
    int id;
};

int main(){
   pthread_t t1, t2, t3;

   pthread_create(&t1, NULL, (void *) atualiza_sensor_esteira1, NULL);
   pthread_create(&t2, NULL, (void *) atualiza_sensor_esteira2, NULL);
   pthread_create(&t3, NULL, (void *) atualiza_display, NULL);

   pthread_join(t1, NULL);
   pthread_join(t2, NULL);
   pthread_join(t3, NULL);

   return 0;
}

// esteira2 possui itens com peso maior ou igual a 5kg, – passa 1 item a cada 2 segundos pelo sensor.
void atualiza_sensor_esteira1(){
    double lido = 0;
    int interval = 2000000;
    while(1){
        pthread_mutex_lock( &exclusao_mutua);
        printf("\nE1 In - Lido 1: %d", (int)lido);
        //printf("\nE1 In - Lido 1: %d", (int)sensor_lido);

        sensor_lido += lido;
        itens_lidos++;

        pthread_mutex_unlock( &exclusao_mutua);
        printf("\nE1 Out - Lido 1: %d", (int)lido);
        // printf("\nE1 Out - Lido 1: %d", (int)sensor_lido);

        usleep(interval);
        lido++;
   }
}

// esteira2 possui itens com peso maior ou igual a 2kg,  passa 1 item a cada 1 segundo pelo sensor.
void atualiza_sensor_esteira2(){
    double lido = 0;
    int interval = 1000000;
    while(1){
        pthread_mutex_lock( &exclusao_mutua);
        printf("\nE2 In - Lido 1: %d", (int)lido);
        // printf("\nE2 In - Lido 1: %d", (int)sensor_lido);

        sensor_lido += lido;
        itens_lidos++;

        pthread_mutex_unlock( &exclusao_mutua);
        printf("\nE2 Out - Lido 1: %d", (int)lido);
        // printf("\nE2 Out - Lido 1: %d", (int)sensor_lido);

        usleep(interval);
        lido++;
   }
}

// atualizar peso total a cada 500 itens
void atualiza_display(){ 
   while(1){
      pthread_mutex_lock( &exclusao_mutua);

      if (itens_lidos % 50 == 0) {
          printf("\nPeso total: %lf", sensor_lido);
      }

      pthread_mutex_unlock( &exclusao_mutua);

      usleep(100000); // Espera 1 segundos
   }
}
