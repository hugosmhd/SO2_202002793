#include <stdio.h>
#include <time.h>

int main() {
    clock_t start, end;
    double cpu_time_used;

    start = clock();

    long long sum = 0;
    int i;

    for (i = 1; i <= 100000; i++) {
        sum += i;
    }

    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

    printf("*** Hugo Sebastian Martínez Hernández - 202002793 ***\n");
    printf("La suma de los números consecutivos de 1 a 100000 es: %lld\n", sum);
    printf("Tiempo de ejecución: %f segundos\n", cpu_time_used);

    return 0;
}