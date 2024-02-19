#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>

#define FILENAME "practica1.txt"
#define NUM_CHILD 2

typedef struct {
    int total_syscalls;
    int read_calls;
    int write_calls;
    int seek_calls;
} Counter;

Counter *counter;

void sigint_handler(int signum) {
    printf("\nSeñal SIGINT recibida. Terminando el programa...\n");
    printf("Número total de llamadas al sistema: %d\n", counter->total_syscalls);
    printf("Read: %d\n", counter->read_calls);
    printf("Write: %d\n", counter->write_calls);
    printf("Seek: %d\n", counter->seek_calls);
    // Liberar la memoria compartida
    shmdt(counter);
    exit(EXIT_SUCCESS);
}

int main() {

    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        perror("Error al establecer el manejador de señales");
        return EXIT_FAILURE;
    }

    printf("** PID proceso padre = %d comienza ** \n", getpid());
    int pid_child[NUM_CHILD];
    int status;
    int fd = open(FILENAME, O_CREAT | O_RDWR | O_TRUNC, 0644);
    close(fd);

    // Obtener el identificador de la memoria compartida
    int shmid = shmget(IPC_PRIVATE, sizeof(Counter), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("Error al obtener el identificador de la memoria compartida");
        return EXIT_FAILURE;
    }

    // Adjuntar la memoria compartida al espacio de direcciones del proceso padre
    counter = (Counter *)shmat(shmid, NULL, 0);
    if (counter == (Counter *)-1) {
        perror("Error al adjuntar la memoria compartida");
        return EXIT_FAILURE;
    }

    // Inicializar los contadores en la memoria compartida
    counter->read_calls = 0;
    counter->write_calls = 0;
    counter->seek_calls = 0;

    // Crear procesos hijos y pasar la dirección de memoria compartida como argumento
    for (int i = 0; i < NUM_CHILD; i++) {
        pid_child[i] = fork();
        if (pid_child[i] == -1) {
            perror("Error en fork");
            exit(EXIT_FAILURE);
        } else if (pid_child[i] == 0) {
            // Proceso hijo
            char shmid_str[20];
            snprintf(shmid_str, 20, "%d", shmid);
            execl("./child.bin", "child.bin", shmid_str, NULL);
            perror("Error al ejecutar el proceso hijo");
            exit(EXIT_FAILURE);
        }
    }

    printf("PID procesos hijos: %d, %d\n", pid_child[0], pid_child[1]);
    char command[100];
    sprintf(command, "%s %d %d %s", "sudo stap trace.stp ", pid_child[0], pid_child[1], " > syscalls.log");
    system(command);

    // Esperar a que todos los hijos terminen
    for (int i = 0; i < NUM_CHILD; i++) {
        wait(NULL);
    }

    // Liberar la memoria compartida
    shmdt(counter);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}
