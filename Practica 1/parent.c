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

#define NUM_CHILD 2

volatile sig_atomic_t stop = 0;

int main(){

    printf("** proc. PID = %d comienza ** \n", getpid());
    int pid;
    int status;

    for (int i = 0; i < NUM_CHILD; i++) {
        pid = fork();
        if (pid == -1) {
            perror("Error en fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // printf("Proceso hijo\n");
            execl("./child.bin", "child.bin", NULL);
            perror("Error al ejecutar el proceso hijo");
            exit(EXIT_FAILURE);
        } else {
            printf("Proceso padre\n");
        }
    }

    while (!stop) {
        sleep(1); // Espera un segundo
    }

    for (int i = 0; i < NUM_CHILD; i++) {
        pid = wait(&status);
        printf("Padre ha terminado PID = %d, hijo de PID = %d terminado, estado = %d \n", getpid(), pid, WEXITSTATUS(status));
    }    

    // for (int i = 0; i < 2; i++) {
    //     int pid1 = fork();
    //     if (pid1 == -1) {
    //         perror("Error en fork");
    //         exit(EXIT_FAILURE);
    //     } else if (pid1 == 0) {
    //         printf("Proceso hijo\n");
    //         monitor_syscalls();
    //         sleep(3);
    //         exit(EXIT_SUCCESS);
    //     } else {
    //         printf("Proceso padre\n");
    //         int status;
    //         wait(&status);

    //         if(WIFEXITED(status)){
    //             printf("El proceso hijo termino con estado: %d\n", WEXITSTATUS(status));
    //         } else {
    //             printf("Ocurrio un error al terminar el proceso hijo\n");
    //         }
    //     }
    // }

    return 0;
}