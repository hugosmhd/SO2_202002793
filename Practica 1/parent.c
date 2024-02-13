#include <stdio.h> // para el printf
#include <string.h>
#include <stdlib.h>
#include <unistd.h> // para usar fork
#include <errno.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <fcntl.h>

void monitor_syscalls() {
    int logfile = open("syscalls.log", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (logfile == -1) {
        perror("No se pudo abrir el archivo de log");
        exit(EXIT_FAILURE);
    }

}

int main(){

    printf("** proc. PID = %d comienza ** \n", getpid());

    for (int i = 0; i < 2; i++) {
        int pid1 = fork();
        if (pid1 == -1) {
            perror("Error en fork");
            exit(EXIT_FAILURE);
        } else if (pid1 == 0) {
            printf("Proceso hijo\n");
            monitor_syscalls();
            sleep(3);
            exit(EXIT_SUCCESS);
        } else {
            printf("Proceso padre\n");
            int status;
            wait(&status);

            if(WIFEXITED(status)){
                printf("El proceso hijo termino con estado: %d\n", WEXITSTATUS(status));
            } else {
                printf("Ocurrio un error al terminar el proceso hijo\n");
            }
        }
    }

    return 0;
}