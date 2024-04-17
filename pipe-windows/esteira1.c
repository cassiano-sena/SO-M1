// Esteira 1
#include <stdio.h>
#include <windows.h>

#define BUFFER_SIZE 1024
#define PESO_ITEM 5

int main()
{
    HANDLE hPipeEsteira1;
    char buffer[BUFFER_SIZE];
    DWORD dwWritten;

    // Open named pipe for conveyor belt 1
    hPipeEsteira1 = CreateFile("\\\\.\\pipe\\esteira1",
                               GENERIC_READ | GENERIC_WRITE,
                               0,
                               NULL,
                               OPEN_EXISTING,
                               0,
                               NULL);

    if (hPipeEsteira1 == INVALID_HANDLE_VALUE)
    {
        printf("Falha em abrir o pipe. Codigo do erro: %d\n", GetLastError());
        return 1;
    }

    printf("Conectado à Tela!\n");

    int itemCount = 0;
    int totalWeight = 0;

    while (1)
    {
        // Simulate passing an item every 2 seconds
        Sleep(2000);

        itemCount++;
        totalWeight += PESO_ITEM;

        sprintf(buffer, "%d|%d", itemCount, totalWeight);

        // Send data to screen
        if (!WriteFile(hPipeEsteira1, buffer, strlen(buffer) + 1, &dwWritten, NULL))
        {
            printf("Falha em escrever no pipe. Codigo do erro: %d\n", GetLastError());
            CloseHandle(hPipeEsteira1);
            return 1;
        }
    }

    // Close named pipe
    CloseHandle(hPipeEsteira1);

    return 0;
}
