#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#define MAX_USERS 105
#define MAX_NAME_LENGTH 50
#define NUM_THREADS 3

struct data_struct {
    int no_cuenta;
    char *nombre;
    float saldo;
};

struct thread_data {
    int thread_id;
    char *filename;
};

struct data_struct usuarios[MAX_USERS];
int num_users = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void create_log_file(int users_per_thread[], int errors[]) {
    time_t current_time;
    struct tm *time_info;
    char time_str[20]; // Suficiente espacio para almacenar la fecha y hora en formato yyyy_MM_dd-HH_mm_ss

    // Obtenemos la fecha y hora actual
    time(&current_time);
    time_info = localtime(&current_time);

    // Formateamos la fecha y hora actual
    strftime(time_str, sizeof(time_str), "%Y_%m_%d-%H_%M_%S", time_info);

    // Creamos el nombre del archivo de registro
    char log_filename[50];
    snprintf(log_filename, sizeof(log_filename), "carga_%s.log", time_str);

    // Creamos el archivo de registro
    FILE *log_file = fopen(log_filename, "w");
    if (log_file == NULL) {
        printf("Error: No se pudo crear el archivo de registro\n");
        return;
    }

    // Escribimos en el archivo de registro
    fprintf(log_file, "------------ Carga de usuarios ------------\n");
    fprintf(log_file, "Fecha: %s\n\n", time_str);
    fprintf(log_file, "Usuarios Cargados:\n");
    for (int i = 0; i < NUM_THREADS; i++) {
        fprintf(log_file, "Hilo #%d: %d\n", i + 1, users_per_thread[i]);
    }
    int total_users = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        total_users += users_per_thread[i];
    }
    fprintf(log_file, "Total: %d\n\n", total_users);
    fprintf(log_file, "Errores:\n");
    for (int i = 0; i < NUM_THREADS; i++) {
        for (int j = 0; j < errors[i]; j++) {
            fprintf(log_file, "    - Linea #%d: Error en carga\n", j + 1);
        }
    }

    // Cerramos el archivo
    fclose(log_file);

    printf("Archivo de registro creado: %s\n", log_filename);
}

void *load_data(void *arg) {
    struct thread_data *data = (struct thread_data *)arg;
    int thread_id = data->thread_id;
    char *filename = data->filename;
    FILE *fp;
    fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Error: No se pudo abrir el archivo\n");
        pthread_exit(NULL);
    }

    char row[MAX_NAME_LENGTH * 2]; // Suponemos que el nombre más largo es de 50 caracteres
    char *token;

    // Avanzamos a la línea inicial correspondiente a este hilo
    for (int i = 0; i < thread_id; i++) {
        if (fgets(row, sizeof(row), fp) == NULL) {
            printf("Error: Archivo de entrada no tiene suficientes datos\n");
            fclose(fp);
            pthread_exit(NULL);
        }
    }

    int users_loaded = 0;
    int errors = 0;

    // Leemos líneas alternas y saltamos las intermedias
    while (fgets(row, sizeof(row), fp)) {
        token = strtok(row, ",");
        int no_cuenta = atoi(token);

        token = strtok(NULL, ",");
        
        char *nombre = strdup(token);
        
        if (nombre == NULL) {
            printf("Error: No se pudo asignar memoria para el nombre\n");
            errors++;
            continue;
        }

        token = strtok(NULL, ",");
        float saldo;
        if (sscanf(token, "%f", &saldo) != 1) {
            printf("Error: Saldo no válido - Nombre: %s, Saldo: %s\n", nombre, token);
            errors++;
            free(nombre);
            continue;
        }

        if (no_cuenta == 0 || strlen(nombre) == 0 || saldo < 0) {
            printf("Error: Registro incorrecto - Nombre: %s, Saldo: %.2f\n", nombre, saldo);
            errors++;
            free(nombre);
            continue;
        }

        pthread_mutex_lock(&mutex);
        usuarios[num_users].no_cuenta = no_cuenta;
        usuarios[num_users].nombre = nombre;
        usuarios[num_users].saldo = saldo;
        num_users++;
        pthread_mutex_unlock(&mutex);

        users_loaded++;

        // Avanzamos al siguiente conjunto de datos correspondiente a este hilo
        for (int i = 1; i < NUM_THREADS; i++) {
            if (fgets(row, sizeof(row), fp) == NULL) {
                break;
            }
        }
    }

    fclose(fp);
    
    // Imprimir estadísticas del hilo
    printf("Hilo #%d: Usuarios cargados: %d, Errores: %d\n", thread_id + 1, users_loaded, errors);
    
    pthread_exit(NULL);
}

void menu() {
    printf("1. Carga masiva de usuarios\n");
    printf("2. Salir\n");
    printf("Ingrese su opcion: ");
}

int main() {
    int option;
    char filename[100];
    pthread_t threads[NUM_THREADS];
    struct thread_data thread_args[NUM_THREADS];
    int users_per_thread[NUM_THREADS] = {0}; // Para llevar cuenta de usuarios cargados por cada hilo
    int errors[NUM_THREADS] = {0}; // Para llevar cuenta de errores por cada hilo

    while (1) {
        menu();
        if (scanf("%d", &option) != 1) {
            printf("Error: Entrada no válida. Intente de nuevo.\n");
            while (getchar() != '\n'); // Limpiar el búfer de entrada
            continue;
        }

        switch (option) {
            case 1:
                printf("Ingrese la ruta del archivo: ");
                scanf("%s", filename);

                // Crear y ejecutar los hilos
                for (int i = 0; i < NUM_THREADS; i++) {
                    thread_args[i].thread_id = i;
                    thread_args[i].filename = strdup(filename); // Copiar la cadena de la ruta del archivo
                    pthread_create(&threads[i], NULL, load_data, (void *)&thread_args[i]);
                }

                // Esperar a que todos los hilos terminen
                for (int i = 0; i < NUM_THREADS; i++) {
                    pthread_join(threads[i], NULL);
                    free(thread_args[i].filename); // Liberar la memoria asignada para la ruta del archivo
                }

                // Calcular estadísticas globales
                for (int i = 0; i < NUM_THREADS; i++) {
                    users_per_thread[i] = 0;
                    errors[i] = 0;
                }

                for (int i = 0; i < NUM_THREADS; i++) {
                    // Sumar usuarios cargados por cada hilo
                    users_per_thread[i] += usuarios[i].no_cuenta;

                    // Sumar errores por cada hilo
                    errors[i] += usuarios[i].no_cuenta;
                }

                // Generar archivo de registro
                create_log_file(users_per_thread, errors);
                break;

            case 2:
                printf("Saliendo del programa.\n");
                return 0;

            default:
                printf("Opcion no valida. Intente de nuevo.\n");
                break;
        }
    }

    return 0;
}
