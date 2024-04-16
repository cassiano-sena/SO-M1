#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

double peso_total_esteira1 = 0;
double peso_total_esteira2 = 0;
double peso_total_combinado = 0;
int itens_lidos = 0;
int atualizacoes_display = 0;

void atualiza_sensor_esteira1(int fd);
void atualiza_sensor_esteira2(int fd);
void atualiza_display(int fd);

int main()
{
    int fd1, fd2, fd3;
    pid_t pid;

    // Criando os pipes nomeados
    mkfifo("/tmp/pipe1", 0666);
    mkfifo("/tmp/pipe2", 0666);
    mkfifo("/tmp/pipe3", 0666);

    // Criando os processos filhos
    pid = fork();

    if (pid < 0) {
        fprintf(stderr, "Falha ao criar o processo filho\n");
        return 1;
    } else if (pid > 0) { // Processo pai
        fd1 = open("/tmp/pipe1", O_WRONLY);
        fd2 = open("/tmp/pipe2", O_WRONLY);
        fd3 = open("/tmp/pipe3", O_RDONLY);

        // Fechando os lados de leitura dos pipes nomeados
        close(fd3);

        atualiza_sensor_esteira1(fd1);
        atualiza_sensor_esteira2(fd2);

        // Fechando os pipes nomeados
        close(fd1);
        close(fd2);
        unlink("/tmp/pipe1");
        unlink("/tmp/pipe2");
    } else { // Processo filho
        fd3 = open("/tmp/pipe3", O_WRONLY);
        atualiza_display(fd3);
        close(fd3);
    }

    return 0;
}

// esteira1 possui itens com peso maior ou igual a 5kg, – passa 1 item a cada 2 segundos pelo sensor.
void atualiza_sensor_esteira1(int fd)
{
    double lido = 0;
    int peso = 5;
    int interval = 2;
    while (1)
    {
        peso_total_esteira1 += peso;
        peso_total_combinado += peso;
        itens_lidos++;
        lido++;

        if (itens_lidos % 50 == 0) {
            // Enviando os dados pelo pipe
            write(fd, &peso_total_esteira1, sizeof(double));
            write(fd, &peso_total_esteira2, sizeof(double));
            write(fd, &peso_total_combinado, sizeof(double));
        }

        sleep(interval);
    }
}

// esteira2 possui itens com peso maior ou igual a 5kg, – passa 1 item a cada 1 segundos pelo sensor.
void atualiza_sensor_esteira2(int fd)
{
    double lido = 0;
    int peso = 2;
    int interval = 1;
    while (1)
    {
        peso_total_esteira2 += peso;
        peso_total_combinado += peso;
        itens_lidos++;
        lido++;

        if (itens_lidos % 50 == 0) {
            // Enviando os dados pelo pipe
            write(fd, &peso_total_esteira1, sizeof(double));
            write(fd, &peso_total_esteira2, sizeof(double));
            write(fd, &peso_total_combinado, sizeof(double));
        }

        sleep(interval);
    }
}

// atualizar peso total a cada 500 itens
void atualiza_display(int fd)
{
    double peso_esteira1, peso_esteira2, peso_combinado;
    while (1)
    {
        // Lendo os dados dos pipes
        read(fd, &peso_esteira1, sizeof(double));
        read(fd, &peso_esteira2, sizeof(double));
        read(fd, &peso_combinado, sizeof(double));

        printf("\nPeso total Esteira 1: %.2lf", peso_esteira1);
        printf("\nPeso total Esteira 2: %.2lf", peso_esteira2);
        printf("\nPeso total combinado: %.2lf", peso_combinado);
    }
}
