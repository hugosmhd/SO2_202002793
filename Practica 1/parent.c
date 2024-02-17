#include <stdio.h> // para el printf
#include <string.h>
#include <stdlib.h>
#include <unistd.h> // para usar fork
#include <errno.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>

#define FILENAME "practica1.txt"
#define NUM_CHILD 2

volatile sig_atomic_t stop = 0;

int main() {

    printf("** proc. PID = %d comienza ** \n", getpid());
    int pid_child[NUM_CHILD];
    int status;
    int fd = open(FILENAME, O_CREAT | O_RDWR | O_TRUNC, 0644);
    close(fd);
    
    for (int i = 0; i < NUM_CHILD; i++) {
        pid_child[i] = fork();
        if (pid_child[i] == -1) {
            perror("Error en fork");
            exit(EXIT_FAILURE);
        } else if (pid_child[i] == 0) {
            printf("Proceso hijo PID:  %d\n", getpid());
            execl("./child.bin", "child.bin", NULL);
            perror("Error al ejecutar el proceso hijo");
            exit(EXIT_FAILURE);
        } else {
            printf("Proceso padre\n");
            
        }
    }

    // Esperar un segundo antes de ejecutar SystemTap
    // sleep(1);
    char command[100];
    printf("Proceso hola: %d, %d\n", pid_child[0], pid_child[1]);
    sprintf(command, "%s %d %d %s", "sudo stap trace.stp ", pid_child[0], pid_child[1], " > syscalls.log");
    system(command);

    while (!stop) {
        sleep(1); // Espera un segundo
    }

    for (int i = 0; i < NUM_CHILD; i++) {
        pid_child[i] = wait(&status);
        printf("Padre ha terminado PID = %d, hijo de PID = %d terminado, estado = %d \n", getpid(), pid_child[i], WEXITSTATUS(status));
    }    

    return 0;
}