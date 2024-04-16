// implementação pipe em linux
// gcc -o pipe pipe.c -lrt
// lrt pra memoria compartilhada
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>
#include <sys/mman.h>

#define PIPE_NAME_ESTEIRA1 "/tmp/esteira1_pipe"
#define PIPE_NAME_ESTEIRA2 "/tmp/esteira2_pipe"
#define PIPE_NAME_DISPLAY "/tmp/display_pipe"

double peso_total_esteira1 = 0;
double peso_total_esteira2 = 0;
double *peso_total_combinado;
int display_threshold = 10; // total de itens lidos para atualizar o display (default 500)
int *itens_lidos;
int atualizacoes_display = 0;
int stop_threshold = 50; // Limite de itens lidos para encerrar (default 1000)

void atualiza_sensor_esteira1();
void atualiza_sensor_esteira2();
void atualiza_display();

int main(){
    setbuf(stdout, NULL); // pra ficar melhor impresso no console
    struct timespec start, end;
    double tempo_execucao;

    clock_gettime(CLOCK_MONOTONIC, &start);

    mkfifo(PIPE_NAME_ESTEIRA1, 0666);
    mkfifo(PIPE_NAME_ESTEIRA2, 0666);
    mkfifo(PIPE_NAME_DISPLAY, 0666);

    // utilizei memoria compartilhada :(
    // memoria compartilhada para obter peso combinado e itens lidos
    int shm_fd = shm_open("/shared_memory", O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, sizeof(double) + sizeof(int));
    peso_total_combinado = mmap(0, sizeof(double), PROT_WRITE, MAP_SHARED, shm_fd, 0);
    itens_lidos = (int *)(peso_total_combinado + 1);
    *peso_total_combinado = 0;
    *itens_lidos = 0;

    pid_t pid_esteira1 = fork();
    if (pid_esteira1 == 0){
        atualiza_sensor_esteira1();
        exit(0);
    }

    pid_t pid_esteira2 = fork();
    if (pid_esteira2 == 0){
        atualiza_sensor_esteira2();
        exit(0);
    }

    pid_t pid_display = fork();
    if (pid_display == 0){
        atualiza_display();
        exit(0);
    }

    waitpid(pid_esteira1, NULL, 0);
    waitpid(pid_esteira2, NULL, 0);
    waitpid(pid_display, NULL, 0);

    unlink(PIPE_NAME_ESTEIRA1);
    unlink(PIPE_NAME_ESTEIRA2);
    unlink(PIPE_NAME_DISPLAY);

    clock_gettime(CLOCK_MONOTONIC, &end);
    tempo_execucao = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("\nTempo de execução total: %.6f segundos\n", tempo_execucao);

    printf("Taxa de atualização do peso total: %.2f atualizações/segundo\n", (float)(*itens_lidos) / tempo_execucao);
    printf("Tempo médio de atualização do display: %.6f segundos\n", tempo_execucao / atualizacoes_display);

    shm_unlink("/shared_memory");
    munmap(peso_total_combinado, sizeof(double));
    munmap(itens_lidos, sizeof(int));

    return 0;
}

void atualiza_sensor_esteira1(){
    // checagem de saúde do pipe pra esteira1
    int fd;
    fd = open(PIPE_NAME_ESTEIRA1, O_WRONLY);
    if (fd == -1){
        perror("Falha ao ler pipe de esteira1");
        exit(1);
    }

    double lido = 0;
    int peso = 5;
    int interval = 2000000; // 2 segundos
    char buffer[1024];

    // enquanto menor que condição de parada, simular esteira1
    while (*itens_lidos < stop_threshold){
        printf("\nEsteira 1 Saída - Lido 1: %d", (int)lido);
        fflush(stdout);
        sprintf(buffer, "%.2lf", peso_total_esteira1);

        peso_total_esteira1 += peso;
        *peso_total_combinado += peso;
        (*itens_lidos)++;

        if (write(fd, buffer, strlen(buffer) + 1) == -1){
            perror("Falha ao escrever no pipe de esteira1");
            close(fd);
            exit(1);
        }
        printf("\nEsteira 1 Saída - Lido 1: %d", (int)lido);
        fflush(stdout);
        usleep(interval);
        lido++;
    }
    close(fd);
}

void atualiza_sensor_esteira2(){
    // checagem de saúde do pipe pra esteira2
    int fd;
    fd = open(PIPE_NAME_ESTEIRA2, O_WRONLY);
    if (fd == -1){
        perror("Falha ao ler pipe de esteira2");
        exit(1);
    }

    double lido = 0;
    int peso = 2;
    int interval = 1000000; // 1 segundo
    char buffer[1024];

    // enquanto menor que condição de parada, simular esteira2
    while (*itens_lidos < stop_threshold){
        sprintf(buffer, "%.2lf", peso_total_esteira2);

        printf("\nEsteira 2 Entrada - Lido 1: %d", (int)lido);
        fflush(stdout);
        peso_total_esteira2 += peso;
        *peso_total_combinado += peso;
        (*itens_lidos)++;

        if (write(fd, buffer, strlen(buffer) + 1) == -1){
            perror("Falha ao escrever no pipe de esteira2");
            close(fd);
            exit(1);
        }
        printf("\nEsteira 2 Saída - Lido 1: %d", (int)lido);
        fflush(stdout);
        usleep(interval);
        lido++;
    }
    close(fd);
}

void atualiza_display(){
    int fd1 = open(PIPE_NAME_ESTEIRA1, O_RDONLY);
    if (fd1 == -1){
        perror("Falha ao ler pipe de esteira1 in display");
        exit(1);
    }

    int fd2 = open(PIPE_NAME_ESTEIRA2, O_RDONLY);
    if (fd2 == -1){
        perror("Falha ao ler pipe de esteira2 in display");
        exit(1);
    }

    char buffer1[1024];
    char buffer2[1024];

    while (*itens_lidos < stop_threshold){
        if (read(fd1, buffer1, sizeof(buffer1)) == -1){
            perror("Failed to read from pipe for esteira1 in display");
            close(fd1);
            exit(1);
        }

        if (read(fd2, buffer2, sizeof(buffer2)) == -1){
            perror("Failed to read from pipe for esteira2 in display");
            close(fd2);
            exit(1);
        }

        fflush(stdout);
        if (*itens_lidos % display_threshold == 0){
            // printf("\n");
            printf("\nItens Lidos: %d", *itens_lidos);
            printf("\nPeso total Esteira 1: %.2lf", atof(buffer1));
            printf("\nPeso total Esteira 2: %.2lf", atof(buffer2));
            printf("\nPeso total combinado: %.2lf", *peso_total_combinado);
            printf("\n");
            atualizacoes_display++;
            fflush(stdout);
        }

        usleep(100000);
    }
    close(fd1);
    close(fd2);
}
