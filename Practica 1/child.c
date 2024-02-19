#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/wait.h>

#define FILENAME "practica1.txt"
#define LINE_LENGTH 8

typedef struct {
    int total_syscalls;
    int read_calls;
    int write_calls;
    int seek_calls;
} Counter;

Counter *counter;

void random_calls(int fd) {
    int operation = rand() % 3; // Operación aleatoria (0: Write, 1: Read, 2: Seek)
    char buffer[LINE_LENGTH + 1]; // Buffer para almacenar datos

    if (operation == 0) {
        for (int i = 0; i < LINE_LENGTH; i++) {
            buffer[i] = 'A' + (rand() % 26);
        }
        write(fd, buffer, strlen(buffer));
        counter->write_calls++;
    } else if (operation == 1) {
        read(fd, buffer, LINE_LENGTH);
        counter->read_calls++;
    } else if (operation == 2) {
        lseek(fd, 0, SEEK_SET); 
        counter->seek_calls++;
    }

    counter->total_syscalls++;
}


int main(int argc, char *argv[]) {
    // Verificar que se haya proporcionado la dirección de memoria compartida
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <shmid>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Convertir la dirección de memoria compartida de cadena a entero
    int shmid = atoi(argv[1]);

    // Adjuntar la memoria compartida al espacio de direcciones del proceso hijo
    counter = (Counter *)shmat(shmid, NULL, 0);
    if (counter == (Counter *)-1) {
        perror("Error al adjuntar la memoria compartida");
        exit(EXIT_FAILURE);
    }

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

    // Incrementar un contador específico
    counter->read_calls++;

    // Desadjuntar la memoria compartida
    shmdt(counter);

    return EXIT_SUCCESS;
}
