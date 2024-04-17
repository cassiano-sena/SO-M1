// Tela
#include <windows.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#define BUFFER_SIZE 1024

int main()
{
    struct timespec start, end;
    double tempo_execucao;
    HANDLE hPipeEsteira1, hPipeEsteira2;
    char buffer[BUFFER_SIZE];
    DWORD dwRead, dwWritten;

    char *token1;
    char *token2;
    int itens_esteira1;
    int peso_esteira1;
    int itens_esteira2;
    int peso_esteira2;
    int itens_lidos = 0;
    int atualizacoes_display = 0;
    int threshold = 10;      // total de itens lidos para atualizar o display
    int stop_threshold = 50; // Threshold para parar o programa
    int multiplicador_quantidade = 1;

    clock_gettime(CLOCK_MONOTONIC, &start);

    // Open named pipes for each conveyor belt
    hPipeEsteira1 = CreateNamedPipe("\\\\.\\pipe\\esteira1",
                                    PIPE_ACCESS_DUPLEX,
                                    PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
                                    1,
                                    BUFFER_SIZE,
                                    BUFFER_SIZE,
                                    0,
                                    NULL);

    hPipeEsteira2 = CreateNamedPipe("\\\\.\\pipe\\esteira2",
                                    PIPE_ACCESS_DUPLEX,
                                    PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
                                    1,
                                    BUFFER_SIZE,
                                    BUFFER_SIZE,
                                    0,
                                    NULL);

    if (hPipeEsteira1 == INVALID_HANDLE_VALUE || hPipeEsteira2 == INVALID_HANDLE_VALUE)
    {
        printf("Falha em abrir o pipe. Codigo do erro: %d\n", GetLastError());
        return 1;
    }

    // Wait for client to connect
    printf("Esperando o cliente conectar...\n");
    if (!ConnectNamedPipe(hPipeEsteira1, NULL))
    {
        printf("Falha em conectar ao cliente. Codigo do erro: %d\n", GetLastError());
        CloseHandle(hPipeEsteira1);
        CloseHandle(hPipeEsteira2);
        return 1;
    }

    printf("Esperando o cliente conectar...\n");
    if (!ConnectNamedPipe(hPipeEsteira2, NULL))
    {
        printf("Falha em conectar ao cliente. Codigo do erro: %d\n", GetLastError());
        CloseHandle(hPipeEsteira1);
        CloseHandle(hPipeEsteira2);
        return 1;
    }

    printf("Conectado às esteiras!\n");

    while (itens_lidos < stop_threshold)
    {
        // Read from conveyor belt 1
        if (ReadFile(hPipeEsteira1, buffer, BUFFER_SIZE, &dwRead, NULL))
        {
            // Dividindo a string recebida em partes individuais
            token1 = strtok(buffer, "|");
            token2 = strtok(NULL, "|");

            // Convertendo os tokens para inteiros
            itens_esteira1 = atoi(token1);
            peso_esteira1 = atoi(token2);
            itens_lidos++;

            printf("E1 - Itens: %d | Peso total: %d Kg\n", itens_esteira1, peso_esteira1);
        }

        // Read from conveyor belt 2
        if (ReadFile(hPipeEsteira2, buffer, BUFFER_SIZE, &dwRead, NULL))
        {
            token1 = strtok(buffer, "|");
            token2 = strtok(NULL, "|");

            itens_esteira2 = atoi(token1);
            peso_esteira2 = atoi(token2);
            itens_lidos += 2;

            printf("E2 - Itens: %d | Peso total: %d Kg\n", itens_esteira2, peso_esteira2);
        }

        int total_itens_lidos = itens_esteira1 + itens_esteira2;
        int peso_total = peso_esteira1 + peso_esteira2;

        printf("T  - Itens: %d | Peso total: %d Kg\n\n", total_itens_lidos, peso_total);

        // Sleep for 2 seconds
        Sleep(2000);

        if (total_itens_lidos >= (multiplicador_quantidade * threshold))
        {
            multiplicador_quantidade++;
            atualizacoes_display++;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    tempo_execucao = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("\nTempo de execucao total: %.6f segundos\n", tempo_execucao);
    printf("Taxa de atualizacao do peso total: %.2f atualizacoes/segundo\n", itens_lidos / tempo_execucao);
    printf("Tempo medio de atualizacao do display: %.6f segundos\n", tempo_execucao / atualizacoes_display);

    // Close named pipes
    CloseHandle(hPipeEsteira1);
    CloseHandle(hPipeEsteira2);

    return 0;
}
