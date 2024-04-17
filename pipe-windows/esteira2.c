// Esteira 2
#include <stdio.h>
#include <windows.h>

#define BUFFER_SIZE 1024
#define PESO_ITEM 2

int main()
{
    HANDLE hPipeEsteira2;
    char buffer[BUFFER_SIZE];
    DWORD dwWritten;

    // Open named pipe for conveyor belt 2
    hPipeEsteira2 = CreateFile("\\\\.\\pipe\\esteira2",
                               GENERIC_READ | GENERIC_WRITE,
                               0,
                               NULL,
                               OPEN_EXISTING,
                               0,
                               NULL);

    if (hPipeEsteira2 == INVALID_HANDLE_VALUE)
    {
        printf("Falha em abrir o pipe. Codigo do erro: %d\n", GetLastError());
        return 1;
    }

    printf("Conectado à Tela!\n");

    int itemCount = 0;
    int totalWeight = 0;

    while (1)
    {
        // Simulate passing an item every 1 second
        Sleep(1000);

        itemCount++;
        totalWeight += PESO_ITEM;

        sprintf(buffer, "%d|%d", itemCount, totalWeight);

        // Send data to screen
        if (!WriteFile(hPipeEsteira2, buffer, strlen(buffer) + 1, &dwWritten, NULL))
        {
            printf("Falha em escrever no pipe. Codigo do erro: %d\n", GetLastError());
            CloseHandle(hPipeEsteira2);
            return 1;
        }
    }

    // Close named pipe
    CloseHandle(hPipeEsteira2);

    return 0;
}
