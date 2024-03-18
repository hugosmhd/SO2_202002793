#include <stdio.h>
#include <pthread.h>
#include <time.h>

#define NUM_THREADS 2
#define MAX_NUM 100000

struct ThreadArgs {
    int start;
    int end;
    long long sum;
};

void *threadSum(void *args) {
    struct ThreadArgs *t_args = (struct ThreadArgs *)args;
    long long sum = 0;
    
    for (int i = t_args->start; i <= t_args->end; i++) {
        sum += i;
    }
    
    t_args->sum = sum;
    pthread_exit(NULL);
}

int main() {
    clock_t start, end;
    double cpu_time_used;

    start = clock();

    pthread_t threads[NUM_THREADS];
    struct ThreadArgs t_args[NUM_THREADS];
    long long total_sum = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        t_args[i].start = i * (MAX_NUM / NUM_THREADS) + 1;
        t_args[i].end = (i + 1) * (MAX_NUM / NUM_THREADS);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("Error al esperar el hilo");
            return 1;
        }
        
        total_sum += t_args[i].sum;
    }

    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

    printf("Hugo Sebastian Martínez Hernández - 202002793");
    printf("La suma de los números consecutivos de 1 a 100000 es: %lld\n", total_sum);
    printf("Tiempo de ejecución: %f segundos\n", cpu_time_used);

    return 0;
}