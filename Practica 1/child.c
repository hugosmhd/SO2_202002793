#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

#define FILENAME "practica1.txt"
#define LINE_LENGTH 8

void random_calls(int fd) {
    // srand(time(NULL));
    int operation = rand() % 3; // Operación aleatoria (0: Write, 1: Read, 2: Seek)
    char buffer[LINE_LENGTH + 1]; // Buffer para almacenar datos

    if (operation == 0) {
        for (int i = 0; i < LINE_LENGTH; i++) {
            buffer[i] = 'A' + (rand() % 26);
        }
        write(fd, buffer, strlen(buffer));
    } else if (operation == 1) {
        read(fd, buffer, LINE_LENGTH);
    } else if (operation == 2) {
        lseek(fd, 0, SEEK_SET); 
    }
}

int main() {
    printf("Hola hijo: %d", getpid());
    int fd = open(FILENAME, O_APPEND | O_RDWR, 0644);

    srand(getpid());
    while (1) {
        if (fd == -1) {
            perror("Error al abrir el archivo");
            exit(EXIT_FAILURE);
        }
        random_calls(fd);
        sleep(rand() % 3 + 1);
    }
    close(fd);

    return 0;
}