// implementação multithread em linux
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

pthread_mutex_t exclusao_mutua = PTHREAD_MUTEX_INITIALIZER; // LOCK da seção crítica

double peso_total_esteira1 = 0;
double peso_total_esteira2 = 0;
double peso_total_combinado = 0;
int itens_lidos = 0;
int atualizacoes_display = 0;
int threshold = 10;      // total de itens lidos para atualizar o display
int stop_threshold = 50; // Limite de itens lidos para encerrar

void atualiza_sensor_esteira1();
void atualiza_sensor_esteira2();
void atualiza_display();
void stop_program();

int main()
{
    pthread_t t1, t2, t3;
    struct timespec start, end;
    double tempo_execucao;

    if (pthread_mutex_init(&exclusao_mutua, NULL) != 0)
    {
        printf("\nFalha na inicialização do mutex!\n");
        return 1;
    }

    clock_gettime(CLOCK_MONOTONIC, &start);

    pthread_create(&t1, NULL, (void *)atualiza_sensor_esteira1, NULL);
    pthread_create(&t2, NULL, (void *)atualiza_sensor_esteira2, NULL);
    pthread_create(&t3, NULL, (void *)atualiza_display, NULL);

    // observador que detecta quando o numero de itens passou do limite estabelecido para encerramento
    while (itens_lidos < stop_threshold)
    {
        usleep(100000);
        // sleep(1);
    }

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    clock_gettime(CLOCK_MONOTONIC, &end);
    tempo_execucao = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("\nTempo de execução total: %.6f segundos\n", tempo_execucao);

    printf("Taxa de atualização do peso total: %.2f atualizações/segundo\n", itens_lidos / tempo_execucao);
    printf("Tempo médio de atualização do display: %.6f segundos\n", tempo_execucao / atualizacoes_display);

    pthread_mutex_destroy(&exclusao_mutua);

    return 0;
}

void atualiza_sensor_esteira1()
{
    double lido = 0;
    int peso = 5;
    int interval = 2000000; // 2 segundos
    while (itens_lidos < stop_threshold)
    {
        pthread_mutex_lock(&exclusao_mutua);
        printf("\nEsteira 1 Entrada - Lido 1: %d", (int)lido);

        peso_total_esteira1 += peso;
        peso_total_combinado += peso;
        itens_lidos++;

        pthread_mutex_unlock(&exclusao_mutua);
        printf("\nEsteira 1 Saída - Lido 1: %d", (int)lido);

        usleep(interval);
        lido++;
    }
}

void atualiza_sensor_esteira2()
{
    double lido = 0;
    int peso = 2;
    int interval = 1000000; // 1 segundo
    while (itens_lidos < stop_threshold)
    {
        pthread_mutex_lock(&exclusao_mutua);
        printf("\nEsteira 2 Entrada - Lido 1: %d", (int)lido);

        peso_total_esteira2 += peso;
        peso_total_combinado += peso;
        itens_lidos++;

        pthread_mutex_unlock(&exclusao_mutua);
        printf("\nEsteira 2 Saída - Lido 1: %d", (int)lido);

        usleep(interval);
        lido++;
    }
}

void atualiza_display()
{
    while (itens_lidos < stop_threshold)
    {
        pthread_mutex_lock(&exclusao_mutua);

        if (itens_lidos % threshold == 0)
        {
            printf("\n");
            printf("\nPeso total Esteira 1: %.2lf", peso_total_esteira1);
            printf("\nPeso total Esteira 2: %.2lf", peso_total_esteira2);
            printf("\nPeso total combinado: %.2lf", peso_total_combinado);
            atualizacoes_display++;
        }

        pthread_mutex_unlock(&exclusao_mutua);

        usleep(100000); // Espera 1 segundos
    }
}
