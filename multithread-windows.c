#include <stdio.h>
#include <windows.h>
#include <time.h>

HANDLE hMutex; // Mutex para exclusão mútua
double peso_total_esteira1 = 0;
double peso_total_esteira2 = 0;
double peso_total_combinado = 0;
int itens_lidos = 0;
int atualizacoes_display = 0;
int threshold = 10;      // total de itens lidos para atualizar o display
int stop_threshold = 50; // Limite de itens lidos para encerrar

DWORD WINAPI atualiza_sensor_esteira1(LPVOID lpParam);
DWORD WINAPI atualiza_sensor_esteira2(LPVOID lpParam);
DWORD WINAPI atualiza_display(LPVOID lpParam);

int main()
{
    struct timespec start, end;
    double tempo_execucao;

    hMutex = CreateMutex(NULL, FALSE, NULL); // Criar o Mutex
    if (hMutex == NULL)
    {
        printf("Falha na criação do mutex\n");
        return 1;
    }

    clock_gettime(CLOCK_MONOTONIC, &start);

    // Criar as threads
    HANDLE hThreadEsteira1 = CreateThread(NULL, 0, atualiza_sensor_esteira1, NULL, 0, NULL);
    HANDLE hThreadEsteira2 = CreateThread(NULL, 0, atualiza_sensor_esteira2, NULL, 0, NULL);
    HANDLE hThreadDisplay = CreateThread(NULL, 0, atualiza_display, NULL, 0, NULL);

    // Esperar até que a condição de encerramento seja atendida
    while (itens_lidos < stop_threshold)
    {
        Sleep(1000); // Esperar 1 segundo
    }

    // Aguardar o término das threads
    WaitForSingleObject(hThreadEsteira1, INFINITE);
    WaitForSingleObject(hThreadEsteira2, INFINITE);
    WaitForSingleObject(hThreadDisplay, INFINITE);

    clock_gettime(CLOCK_MONOTONIC, &end);
    tempo_execucao = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("\nTempo de execução total: %.6f segundos\n", tempo_execucao);

    printf("Taxa de atualização do peso total: %.2f atualizações/segundo\n", (float)itens_lidos / tempo_execucao);
    printf("Tempo médio de atualização do display: %.6f segundos\n", tempo_execucao / atualizacoes_display);

    // Fechar o Mutex
    CloseHandle(hMutex);

    return 0;
}

// Função para atualizar o sensor da esteira 1
DWORD WINAPI atualiza_sensor_esteira1(LPVOID lpParam)
{
    double lido = 0;
    int peso = 5;
    int interval = 2000; // 2 segundos

    while (itens_lidos < stop_threshold)
    {
        WaitForSingleObject(hMutex, INFINITE); // Aguardar o Mutex
        printf("\nEsteira 1 Entrada - Lido 1: %d", (int)lido);

        peso_total_esteira1 += peso;
        peso_total_combinado += peso;
        itens_lidos++;

        ReleaseMutex(hMutex); // Liberar o Mutex
        printf("\nEsteira 1 Saída - Lido 1: %d", (int)lido);

        Sleep(interval);
        lido++;
    }

    return 0;
}

// Função para atualizar o sensor da esteira 2
DWORD WINAPI atualiza_sensor_esteira2(LPVOID lpParam)
{
    double lido = 0;
    int peso = 2;
    int interval = 1000; // 1 segundo

    while (itens_lidos < stop_threshold)
    {
        WaitForSingleObject(hMutex, INFINITE); // Aguardar o Mutex
        printf("\nEsteira 2 Entrada - Lido 1: %d", (int)lido);

        peso_total_esteira2 += peso;
        peso_total_combinado += peso;
        itens_lidos++;

        ReleaseMutex(hMutex); // Liberar o Mutex
        printf("\nEsteira 2 Saída - Lido 1: %d", (int)lido);

        Sleep(interval);
        lido++;
    }

    return 0;
}

// Função para atualizar o display
DWORD WINAPI atualiza_display(LPVOID lpParam)
{
    while (itens_lidos < stop_threshold)
    {
        WaitForSingleObject(hMutex, INFINITE); // Aguardar o Mutex

        if (itens_lidos % threshold == 0)
        {
            printf("\n");
            printf("\nPeso total Esteira 1: %.2lf", peso_total_esteira1);
            printf("\nPeso total Esteira 2: %.2lf", peso_total_esteira2);
            printf("\nPeso total combinado: %.2lf", peso_total_combinado);
            atualizacoes_display++;
        }

        ReleaseMutex(hMutex); // Liberar o Mutex

        Sleep(1000); // Esperar 1 segundo
    }

    return 0;
}
