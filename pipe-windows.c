#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>

#define PIPE_NAME_ESTEIRA1 "\\\\.\\pipe\\esteira1_pipe"
#define PIPE_NAME_ESTEIRA2 "\\\\.\\pipe\\esteira2_pipe"
#define PIPE_NAME_DISPLAY "\\\\.\\pipe\\display_pipe"

double peso_total_esteira1 = 0;
double peso_total_esteira2 = 0;
double peso_total_combinado = 0;
int display_threshold = 10; // total de itens lidos para atualizar o display
int itens_lidos = 0;
int atualizacoes_display = 0;
int stop_threshold = 50; // Limite de itens lidos para encerrar

void atualiza_sensor_esteira1();
void atualiza_sensor_esteira2();
void atualiza_display();

int main()
{
    struct timespec start, end;
    double tempo_execucao;

    clock_gettime(CLOCK_MONOTONIC, &start);

    HANDLE hPipeEsteira1, hPipeEsteira2, hPipeDisplay;
    DWORD dwBytesWritten;
    char buffer[1024];

    hPipeEsteira1 = CreateNamedPipe(PIPE_NAME_ESTEIRA1, PIPE_ACCESS_OUTBOUND, PIPE_TYPE_BYTE, 1, 0, 0, 0, NULL);
    hPipeEsteira2 = CreateNamedPipe(PIPE_NAME_ESTEIRA2, PIPE_ACCESS_OUTBOUND, PIPE_TYPE_BYTE, 1, 0, 0, 0, NULL);
    hPipeDisplay = CreateNamedPipe(PIPE_NAME_DISPLAY, PIPE_ACCESS_INBOUND, PIPE_TYPE_BYTE, 1, 0, 0, 0, NULL);

    ConnectNamedPipe(hPipeEsteira1, NULL);
    ConnectNamedPipe(hPipeEsteira2, NULL);
    ConnectNamedPipe(hPipeDisplay, NULL);

    HANDLE hThreadEsteira1 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)atualiza_sensor_esteira1, NULL, 0, NULL);
    HANDLE hThreadEsteira2 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)atualiza_sensor_esteira2, NULL, 0, NULL);
    HANDLE hThreadDisplay = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)atualiza_display, NULL, 0, NULL);

    WaitForSingleObject(hThreadEsteira1, INFINITE);
    WaitForSingleObject(hThreadEsteira2, INFINITE);
    WaitForSingleObject(hThreadDisplay, INFINITE);

    CloseHandle(hPipeEsteira1);
    CloseHandle(hPipeEsteira2);
    CloseHandle(hPipeDisplay);

    clock_gettime(CLOCK_MONOTONIC, &end);
    tempo_execucao = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("\nTempo de execução total: %.6f segundos\n", tempo_execucao);

    printf("Taxa de atualização do peso total: %.2f atualizações/segundo\n", (float)itens_lidos / tempo_execucao);
    printf("Tempo médio de atualização do display: %.6f segundos\n", tempo_execucao / atualizacoes_display);

    return 0;
}

DWORD WINAPI atualiza_sensor_esteira1(LPVOID lpParam)
{
    HANDLE hPipeEsteira1 = CreateFile(PIPE_NAME_ESTEIRA1, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hPipeEsteira1 == INVALID_HANDLE_VALUE)
    {
        printf("Failed to open pipe for esteira1");
        return 1;
    }

    int peso = 5;
    int interval = 2000; // 2 segundos

    while (itens_lidos < stop_threshold)
    {
        sprintf(buffer, "%.2lf", peso_total_esteira1);

        peso_total_esteira1 += peso;
        peso_total_combinado += peso;
        itens_lidos++;

        WriteFile(hPipeEsteira1, buffer, strlen(buffer) + 1, &dwBytesWritten, NULL);
        Sleep(interval);
    }

    CloseHandle(hPipeEsteira1);
    return 0;
}

DWORD WINAPI atualiza_sensor_esteira2(LPVOID lpParam)
{
    HANDLE hPipeEsteira2 = CreateFile(PIPE_NAME_ESTEIRA2, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hPipeEsteira2 == INVALID_HANDLE_VALUE)
    {
        printf("Failed to open pipe for esteira2");
        return 1;
    }

    int peso = 2;
    int interval = 1000; // 1 segundo

    while (itens_lidos < stop_threshold)
    {
        sprintf(buffer, "%.2lf", peso_total_esteira2);

        peso_total_esteira2 += peso;
        peso_total_combinado += peso;
        itens_lidos++;

        WriteFile(hPipeEsteira2, buffer, strlen(buffer) + 1, &dwBytesWritten, NULL);
        Sleep(interval);
    }

    CloseHandle(hPipeEsteira2);
    return 0;
}

DWORD WINAPI atualiza_display(LPVOID lpParam)
{
    HANDLE hPipeDisplay = CreateFile(PIPE_NAME_DISPLAY, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hPipeDisplay == INVALID_HANDLE_VALUE)
    {
        printf("Failed to open pipe for display");
        return 1;
    }

    char buffer[1024];

    while (itens_lidos < stop_threshold)
    {
        ReadFile(hPipeDisplay, buffer, sizeof(buffer), &dwBytesWritten, NULL);

        if (itens_lidos % display_threshold == 0)
        {
            printf("\nItens Lidos: %d", itens_lidos);
            printf("\nPeso total Esteira 1: %.2lf", peso_total_esteira1);
            printf("\nPeso total Esteira 2: %.2lf", peso_total_esteira2);
            printf("\nPeso total combinado: %.2lf", peso_total_combinado);
            printf("\n");
            atualizacoes_display++;
        }

        Sleep(100);
    }

    CloseHandle(hPipeDisplay);
    return 0;
}
