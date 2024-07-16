#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>

#define MAX_LINE_LENGTH 100 //
#define MAX_USERS 1000
#define MAX_NAME_LENGTH 50
#define NUM_THREADS 3
#define NUM_THREADS_OP 4

// Estructura para almacenar datos del hilo
typedef struct {
    FILE* file;
    sem_t* semaphore;
    int thread_id;
    struct User *users;
    int *num_users;
    struct Error *errors;
    int *users_loaded;
    int *num_errors;
    int operations_count[3];
} ThreadData;

struct User {
    int no_cuenta;
    char nombre[MAX_NAME_LENGTH];
    float saldo;
};

struct Error {
    int line_number;
    char description[100];
};

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

int deposit_count = 0;
int withdrawal_count = 0;
int transfer_count = 0;

// Función para leer una línea del archivo
void* load_data_users(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    int thread_id = data->thread_id;
    struct User *users = data->users;
    int *num_users = data->num_users;
    int *num_errors = data->num_errors;
    int *users_loaded = data->users_loaded;
    struct Error *errors = data->errors;
    char line[MAX_LINE_LENGTH];

    int line_count = 0;
    int users_loaded_count = 0;
    while (1) {
        sem_wait(data->semaphore); // Esperar a que el semáforo esté disponible
        int line_number = data->thread_id + 1; // Calcular el número de línea a leer
        // printf("Hilo %d: Línea %d: %s\n", data->thread_id, line_number, line);
        if (fgets(line, sizeof(line), data->file) != NULL) {
            pthread_mutex_lock(&mutex);
            char *token = strtok(line, ",");
            int no_cuenta = atoi(token);

            token = strtok(NULL, ",");
            if (token == NULL || strlen(token) == 0) {
                snprintf(errors[*num_errors].description, sizeof(errors[*num_errors].description), "Error en la línea %d: Saldo no válido - %s", line_count + 1, token);
                errors[*num_errors].line_number = line_count + 1;
                (*num_errors)++;
                pthread_mutex_unlock(&mutex);
                sem_post(data->semaphore); // Liberar el semáforo para el siguiente hilo
                line_count++;
                continue;
            } 
            strncpy(users[*num_users].nombre, token, MAX_NAME_LENGTH);

            token = strtok(NULL, ",");
            float saldo;
            if (sscanf(token, "%f", &saldo) != 1) {
                snprintf(errors[*num_errors].description, sizeof(errors[*num_errors].description), "Error en la línea %d: Saldo no válido - %s", 1, token);
                // Eliminar el salto de línea manualmente después de la cadena generada
                size_t len = strlen(errors[*num_errors].description);
                if (len > 0 && errors[*num_errors].description[len - 1] == '\n') {
                    errors[*num_errors].description[len - 1] = '\0';
                }
                (*num_errors)++;
                line_count++;
                pthread_mutex_unlock(&mutex);
                sem_post(data->semaphore); // Liberar el semáforo para el siguiente hilo
                continue;
            }
            int usuario_existente = 0;
            for (int i = 0; i < *num_users; i++) {
                if (users[i].no_cuenta == no_cuenta) {
                    usuario_existente = 1;
                    break;
                }
            }

            if (usuario_existente) {
                snprintf(errors[*num_errors].description, sizeof(errors[*num_errors].description), "Error en la línea %d: Numero cuenta duplicado", line_count + 1);
                errors[*num_errors].line_number = line_count + 1;
                (*num_errors)++;
            } else if (sscanf(token, "%f", &saldo) != 1) {
                snprintf(errors[*num_errors].description, sizeof(errors[*num_errors].description), "Error en la línea %d: Saldo inválido", line_count + 1);
                errors[*num_errors].line_number = line_count + 1;
                (*num_errors)++;
            } else if (saldo < 0) {
                snprintf(errors[*num_errors].description, sizeof(errors[*num_errors].description), "Error en la línea %d: Saldo negativo", line_count + 1);
                errors[*num_errors].line_number = line_count + 1;
                (*num_errors)++;
            } else {
                users[*num_users].no_cuenta = no_cuenta;
                users[*num_users].saldo = saldo;
                (*num_users)++;
                users_loaded_count++;
            }
            users_loaded[thread_id] = users_loaded_count;
            pthread_mutex_unlock(&mutex);
        } else {
            line_count++;
            sem_post(data->semaphore); // Liberar el semáforo antes de salir del bucle
            break; // Salir del bucle si no hay más líneas que leer
        }
        line_count++;
        sem_post(data->semaphore); // Liberar el semáforo para el siguiente hilo
    }

    pthread_exit(NULL);
}

void load_users(struct User users[], int *num_users, struct Error errors[], int *num_errors, int *users_loaded) {
    FILE* file = fopen("usuarios.csv", "r");
    if (file == NULL) {
        perror("Error al abrir el archivo");
        // return EXIT_FAILURE;
        return;
    }

    pthread_t threads[NUM_THREADS];
    ThreadData thread_data[NUM_THREADS];

    sem_t semaphore;
    sem_init(&semaphore, 0, 1); // Inicializar el semáforo con un contador de 1 (mutex)

    char header[MAX_LINE_LENGTH];
    fgets(header, MAX_LINE_LENGTH, file);


    *num_users = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        users_loaded[i] = 0;
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        thread_data[i].file = file;
        thread_data[i].semaphore = &semaphore;
        thread_data[i].thread_id = i;
        thread_data[i].thread_id = i;
        thread_data[i].users = users;
        thread_data[i].num_users = num_users;
        thread_data[i].users_loaded = users_loaded;
        thread_data[i].errors = errors;
        thread_data[i].num_errors = num_errors;
        pthread_create(&threads[i], NULL, load_data_users, (void*)&thread_data[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    fclose(file);
    sem_destroy(&semaphore);
}

void create_log_file(int num_users, struct Error errors[], int num_errors, int users_loaded[]) {
    time_t current_time;
    struct tm *time_info;
    char time_str[20];

    time(&current_time);
    time_info = localtime(&current_time);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", time_info);

    char log_filename[50];
    snprintf(log_filename, sizeof(log_filename), "carga_%s.log", time_str);

    FILE *log_file = fopen(log_filename, "w");
    if (log_file == NULL) {
        printf("Error: No se pudo crear el archivo de registro\n");
        return;
    }

    fprintf(log_file, "------------ Carga de usuarios ------------\n");
    fprintf(log_file, "Fecha: %s\n\n", time_str);
    fprintf(log_file, "Usuarios Cargados:\n");
    int total_loaded = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        fprintf(log_file, "Hilo #%d: %d\n", i + 1, users_loaded[i]);
        total_loaded += users_loaded[i];
    }
    fprintf(log_file, "Total: %d\n\n", num_users);
    fprintf(log_file, "Errores:\n");
    if (num_errors == 0) {
        fprintf(log_file, "    - Ningún error encontrado\n");
    } else {
        for (int i = 0; i < num_errors; i++) {
            fprintf(log_file, "    - %s\n", errors[i].description);
        }
    }

    fclose(log_file);
}

// Función para leer una línea del archivo
void* load_data_operations(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    int thread_id = data->thread_id;
    struct User *users = data->users;
    int num_users = *(data->num_users);
    int *num_errors = data->num_errors;
    int *users_loaded = data->users_loaded;
    struct Error *errors = data->errors;
    char line[MAX_LINE_LENGTH];

    int line_count = 0;
    int users_loaded_count = 0;
    while (1) {
        sem_wait(data->semaphore); // Esperar a que el semáforo esté disponible
        int line_number = data->thread_id + 1; // Calcular el número de línea a leer
        if (fgets(line, sizeof(line), data->file) != NULL) {
            // printf("Hilo %d: Línea %d: %s", data->thread_id, line_number, line);
            pthread_mutex_lock(&mutex2);
            char *token = strtok(line, ",");
            int operacion = atoi(token);

            token = strtok(NULL, ",");
            int cuenta1 = atoi(token);

            token = strtok(NULL, ",");
            int cuenta2= atoi(token);

            token = strtok(NULL, ",");
            float monto;
            if (sscanf(token, "%f", &monto) != 1) {
                snprintf(errors[*num_errors].description, sizeof(errors[*num_errors].description), "Error en la línea %d: Monto no válido - %s", 1, token);
                // Eliminar el salto de línea manualmente después de la cadena generada
                size_t len = strlen(errors[*num_errors].description);
                if (len > 0 && errors[*num_errors].description[len - 1] == '\n') {
                    errors[*num_errors].description[len - 1] = '\0';
                }
                (*num_errors)++;
                line_count++;
                pthread_mutex_unlock(&mutex2);
                sem_post(data->semaphore); // Liberar el semáforo para el siguiente hilo
                continue;
            }
            
            // printf("Hilo %d: Ope %d, cue1 %d, cue2 %d, monto %f\n", data->thread_id, operacion, cuenta1, cuenta2, monto);

            int cuenta1_existente = -1;
            int cuenta2_existente = -1;
            for (int i = 0; i < num_users; i++) {
                // printf("Cuenta %d, Nombre %s, Saldo %f \n", users[i].no_cuenta, users[i].nombre, users[i].saldo);
                if (users[i].no_cuenta == cuenta1) {
                    cuenta1_existente = i;
                    if (cuenta2_existente != -1) break;
                } else if (users[i].no_cuenta == cuenta2) {
                    cuenta2_existente = i;
                    if (cuenta1_existente != -1) break;
                }
            }

            if (cuenta1_existente == -1) {
                // printf("cuenta1 no existe %d: Ope %d\n", cuenta1, operacion);
                snprintf(errors[*num_errors].description, sizeof(errors[*num_errors].description), "Error en la línea %d: Numero cuenta1 no existe %d", line_count + 1, cuenta1);
                errors[*num_errors].line_number = line_count + 1;
                (*num_errors)++;
            } else if (cuenta2_existente == -1 && operacion == 3) {
                // printf("cuenta2 no existe %d: Ope %d\n", cuenta2, operacion);
                snprintf(errors[*num_errors].description, sizeof(errors[*num_errors].description), "Error en la línea %d: Numero cuenta2 no existe %d", line_count + 1, cuenta2);
                errors[*num_errors].line_number = line_count + 1;
                (*num_errors)++;
            }  else if (monto < 0) {
                // printf("Monto negativo %f: Ope %d\n", monto, operacion);
                snprintf(errors[*num_errors].description, sizeof(errors[*num_errors].description), "Error en la línea %d: Monto negativo", line_count + 1);
                errors[*num_errors].line_number = line_count + 1;
                (*num_errors)++;
            } else if (operacion > 3 || operacion < 1) {
                // printf("Operacion invalida: Ope %d\n", operacion);
                snprintf(errors[*num_errors].description, sizeof(errors[*num_errors].description), "Error en la línea %d: Operacion no valida", line_count + 1);
                errors[*num_errors].line_number = line_count + 1;
                (*num_errors)++;
            } else if (operacion == 1) { // OPERACION DEPOSITO
                users[cuenta1_existente].saldo += monto;
                users_loaded_count++;
                data->operations_count[0]++;
            } else if (operacion == 2) { // OPERACION RETIRO
                if (users[cuenta1_existente].saldo >= monto) {
                    users[cuenta1_existente].saldo -= monto;
                    users_loaded_count++;
                    data->operations_count[1]++;
                } else {
                    snprintf(errors[*num_errors].description, sizeof(errors[*num_errors].description), "Error en la línea %d: Saldo insuficiente para retirar", line_count + 1);
                    errors[*num_errors].line_number = line_count + 1;
                    (*num_errors)++;
                }
            } else if (operacion == 3) { // OPERACION TRANSFERENCIA
                if (users[cuenta1_existente].saldo >= monto) {
                    users[cuenta1_existente].saldo -= monto;
                    users[cuenta2_existente].saldo += monto;
                    users_loaded_count++;
                    data->operations_count[2]++;
                } else {
                    snprintf(errors[*num_errors].description, sizeof(errors[*num_errors].description), "Error en la línea %d: Saldo insuficiente para retirar", line_count + 1);
                    errors[*num_errors].line_number = line_count + 1;
                    (*num_errors)++;
                }
            }
            
            users_loaded[thread_id] = users_loaded_count;
            pthread_mutex_unlock(&mutex2);
        } else {
            line_count++;
            sem_post(data->semaphore); // Liberar el semáforo antes de salir del bucle
            break; // Salir del bucle si no hay más líneas que leer
        }
        line_count++;
        sem_post(data->semaphore); // Liberar el semáforo para el siguiente hilo
    }

    pthread_exit(NULL);
}

void load_operations(struct User users[], int *num_users, struct Error errors[], int *num_errors, int *users_loaded) {
    FILE* file = fopen("datos.csv", "r");
    if (file == NULL) {
        perror("Error al abrir el archivo");
        // return EXIT_FAILURE;
        return;
    }

    pthread_t threads[NUM_THREADS_OP];
    ThreadData thread_data[NUM_THREADS_OP];

    sem_t semaphore;
    sem_init(&semaphore, 0, 1); // Inicializar el semáforo con un contador de 1 (mutex)

    char header[MAX_LINE_LENGTH];
    fgets(header, MAX_LINE_LENGTH, file);


    for (int i = 0; i < NUM_THREADS_OP; i++) {
        users_loaded[i] = 0;
        for (int j = 0; j < 3; j++) {
            thread_data[i].operations_count[j] = 0; // Inicializar el contador
        }
    }

    for (int i = 0; i < NUM_THREADS_OP; i++) {
        thread_data[i].file = file;
        thread_data[i].semaphore = &semaphore;
        thread_data[i].thread_id = i;
        thread_data[i].thread_id = i;
        thread_data[i].users = users;
        thread_data[i].num_users = num_users;
        thread_data[i].users_loaded = users_loaded;
        thread_data[i].errors = errors;
        thread_data[i].num_errors = num_errors;
        pthread_create(&threads[i], NULL, load_data_operations, (void*)&thread_data[i]);
    }

    for (int i = 0; i < NUM_THREADS_OP; i++) {
        pthread_join(threads[i], NULL);
    }

    for (int i = 0; i < NUM_THREADS_OP; i++) {
            // Suponiendo que tienes una variable thread_data de tipo ThreadData
        deposit_count += thread_data[i].operations_count[0]; // Contador de depósitos
        withdrawal_count += thread_data[i].operations_count[1]; // Contador de retiros
        transfer_count += thread_data[i].operations_count[2]; // Contador de transferencias
    }


    fclose(file);
    sem_destroy(&semaphore);
}

void create_log_file_op(int num_users, struct Error errors[], int num_errors, int users_loaded[]) {
    time_t current_time;
    struct tm *time_info;
    char time_str[20];

    time(&current_time);
    time_info = localtime(&current_time);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", time_info);

    char log_filename[50];
    snprintf(log_filename, sizeof(log_filename), "operaciones_%s.log", time_str);

    FILE *log_file = fopen(log_filename, "w");
    if (log_file == NULL) {
        printf("Error: No se pudo crear el archivo de registro\n");
        return;
    }

    fprintf(log_file, "------------ Resumen de operaciones ------------\n");
    fprintf(log_file, "Fecha: %s\n\n", time_str);
    fprintf(log_file, "Operaciones realizadas:\n");
    fprintf(log_file, "Retiros: %d\n", withdrawal_count);
    fprintf(log_file, "Depositos: %d\n", deposit_count);
    fprintf(log_file, "Transferencias: %d\n", transfer_count);
    fprintf(log_file, "Total: %d\n\n", withdrawal_count + deposit_count + transfer_count);
    fprintf(log_file, "Operaciones por hilo:\n");
    int total_loaded = 0;
    for (int i = 0; i < NUM_THREADS_OP; i++) {
        fprintf(log_file, "Hilo #%d: %d\n", i + 1, users_loaded[i]);
        total_loaded += users_loaded[i];
    }
    fprintf(log_file, "Total: %d\n\n", total_loaded);
    fprintf(log_file, "Errores:\n");
    if (num_errors == 0) {
        fprintf(log_file, "    - Ningún error encontrado\n");
    } else {
        for (int i = 0; i < num_errors; i++) {
            fprintf(log_file, "    - %s\n", errors[i].description);
        }
    }

    fclose(log_file);
}

void create_csv(struct User users[], int num_users) {
    FILE *csv_file = fopen("estados_de_cuentas.csv", "w");
    
    if (csv_file == NULL) {
        printf("Error al abrir el archivo.\n");
        return; // Salir del programa con código de error
    }

    fprintf(csv_file, "no_cuenta,nombre,saldo\n");

    for (int i = 0; i < num_users; i++) {
        fprintf(csv_file, "%d,%s,%.2f\n", users[i].no_cuenta, users[i].nombre, users[i].saldo);
    }

    fclose(csv_file);

    printf("Archivo CSV creado correctamente.\n");
}

void retiro(struct User users[], int num_users) {
    int numero_cuenta;
    float monto;
    
    printf("Ingrese el número de cuenta: ");
    if (scanf("%d", &numero_cuenta) != 1) {
        printf("Error: Número de cuenta no válido.\n");
        while (getchar() != '\n');
        return;
    }

    if (numero_cuenta <= 0) {
        printf("Error: El número de cuenta debe ser positivo.\n");
        return;
    }
    
    printf("Ingrese el monto a retirar: ");
    if (scanf("%f", &monto) != 1) {
        printf("Error: Monto no válido.\n");
        // Limpiar el búfer de entrada
        while (getchar() != '\n');
        return;
    }

    int cuenta1_existente = -1;
    for (int i = 0; i < num_users; i++) {
        if (users[i].no_cuenta == numero_cuenta) {
            cuenta1_existente = i;
            break;
        }
    }

    if (cuenta1_existente == -1) {
        printf("Error cuenta %d no existe\n", numero_cuenta);
        return;
    } else if (monto < 0) {
        printf("Error monto negativo %f\n", monto);
        return;
    } else if (users[cuenta1_existente].saldo < monto) {
        printf("Error saldo insuficiente para retirar %f\n", monto);
        return;
    } else if (users[cuenta1_existente].saldo >= monto) {
        users[cuenta1_existente].saldo -= monto;
    } 

    // Aquí puedes implementar la lógica para realizar el depósito
    printf("Se retiraron %.2f de la cuenta %d.\n", monto, numero_cuenta);
}

void deposito(struct User users[], int num_users) {
    int numero_cuenta;
    float monto;
    
    printf("Ingrese el número de cuenta: ");
    if (scanf("%d", &numero_cuenta) != 1) {
        printf("Error: Número de cuenta no válido.\n");
        while (getchar() != '\n');
        return;
    }

    if (numero_cuenta <= 0) {
        printf("Error: El número de cuenta debe ser positivo.\n");
        return;
    }
    
    printf("Ingrese el monto a depositar: ");
    if (scanf("%f", &monto) != 1) {
        printf("Error: Monto no válido.\n");
        // Limpiar el búfer de entrada
        while (getchar() != '\n');
        return;
    }

    int cuenta1_existente = -1;
    for (int i = 0; i < num_users; i++) {
        if (users[i].no_cuenta == numero_cuenta) {
            cuenta1_existente = i;
            break;
        }
    }

    if (cuenta1_existente == -1) {
        printf("Error cuenta %d no existe\n", numero_cuenta);
        return;
    } else if (monto < 0) {
        printf("Error monto negativo %f\n", monto);
        return;
    } 

    users[cuenta1_existente].saldo += monto;
    // Aquí puedes implementar la lógica para realizar el depósito
    printf("Se depositaron %.2f en la cuenta %d.\n", monto, numero_cuenta);
}

void transferencia(struct User users[], int num_users) {
    int numero_cuenta1;
    int numero_cuenta2;
    float monto;
    
    printf("Ingrese el número de cuenta a retirar: ");
    if (scanf("%d", &numero_cuenta1) != 1) {
        printf("Error: Número de cuenta no válido.\n");
        while (getchar() != '\n');
        return;
    }

    if (numero_cuenta1 <= 0) {
        printf("Error: El número de cuenta debe ser positivo.\n");
        return;
    }

     printf("Ingrese el número de cuenta a depositar: ");
    if (scanf("%d", &numero_cuenta2) != 1) {
        printf("Error: Número de cuenta no válido.\n");
        while (getchar() != '\n');
        return;
    }

    if (numero_cuenta2 <= 0) {
        printf("Error: El número de cuenta debe ser positivo.\n");
        return;
    }
    
    printf("Ingrese el monto a depositar: ");
    if (scanf("%f", &monto) != 1) {
        printf("Error: Monto no válido.\n");
        while (getchar() != '\n');
        return;
    }

    int cuenta1_existente = -1;
    int cuenta2_existente = -1;
    for (int i = 0; i < num_users; i++) {
        // printf("Cuenta %d, Nombre %s, Saldo %f \n", users[i].no_cuenta, users[i].nombre, users[i].saldo);
        if (users[i].no_cuenta == numero_cuenta1) {
            cuenta1_existente = i;
            if (cuenta2_existente != -1) break;
        } else if (users[i].no_cuenta == numero_cuenta2) {
            cuenta2_existente = i;
            if (cuenta1_existente != -1) break;
        }
    }

    if (cuenta1_existente == -1) {
        printf("Error cuenta a retorar %d no existe\n", numero_cuenta1);
        return;
    } else if (cuenta2_existente == -1) {
        printf("Error cuenta a depositar %d no existe\n", numero_cuenta2);
        return;
    } else if (monto < 0) {
        printf("Error monto negativo %f\n", monto);
        return;
    } else {
        if (users[cuenta1_existente].saldo >= monto) {
            users[cuenta1_existente].saldo -= monto;
            users[cuenta2_existente].saldo += monto;
        } else {
            printf("Error Saldo insuficiente para transeferir");
        }
    }

    // Aquí puedes implementar la lógica para realizar el depósito
    printf("Se retiraron %.2f de la cuenta %d y se depositaron en la cuenta %d.\n", monto, numero_cuenta1, numero_cuenta2);
}

void consultar(struct User users[], int num_users) {
    int numero_cuenta1;
    
    printf("Ingrese el número de cuenta a consultar: ");
    if (scanf("%d", &numero_cuenta1) != 1) {
        printf("Error: Número de cuenta no válido.\n");
        while (getchar() != '\n');
        return;
    }

    if (numero_cuenta1 <= 0) {
        printf("Error: El número de cuenta debe ser positivo.\n");
        return;
    }

    int cuenta1_existente = -1;
    for (int i = 0; i < num_users; i++) {
        if (users[i].no_cuenta == numero_cuenta1) {
            printf("Cuenta %d, Nombre %s, Saldo %.2f \n", users[i].no_cuenta, users[i].nombre, users[i].saldo);
            cuenta1_existente = i;
            break;
        }
    }

    if (cuenta1_existente == -1) {
        printf("Error cuenta a retorar %d no existe\n", numero_cuenta1);
        return;
    }
}

void operaciones_individuales(struct User users[], int num_users) {
    int option;
    do {
        printf("\n------------ Operaciones Individuales ------------\n");
        printf("1. Depósito\n");
        printf("2. Retiro\n");
        printf("3. Transferencia\n");
        printf("4. Consultar cuenta\n");
        printf("5. Volver al menú principal\n");
        printf("----------------------------------------------------\n");
        printf("Seleccione una opción: ");
        if (scanf("%d", &option) != 1) {
            printf("Error: Entrada no válida. Intente de nuevo.\n");
            while (getchar() != '\n'); // Limpiar el búfer de entrada
            continue;
        }
        getchar();
        switch (option) {
            case 1:
                // Código para depósito
                printf("***** Depósito *****\n");
                deposito(users, num_users);
                getchar();
                break;
            case 2:
                // Código para retiro
                printf("***** Retiro *****\n");
                retiro(users, num_users);
                getchar();
                break;
            case 3:
                // Código para transferencia
                printf("***** Transferencia *****\n");
                transferencia(users, num_users);
                getchar();
                break;
            case 4:
                // Código para consultar cuenta
                printf("***** Consultar cuenta *****\n");
                consultar(users, num_users);
                getchar();
                break;
            case 5:
                // Volver al menú principal
                printf("Volviendo al menú principal.\n");
                break;
            default:
                printf("Opción no válida. Intente de nuevo.\n");
                break;
        }
        printf("Presiona Enter para continuar...");
        getchar(); // Espera a que el usuario presione Enter

        system("clear"); // Limpia la pantalla (para sistemas Unix/Linux)
    } while (option != 5);
}

void menu() {
    printf("\n------------ Menú ------------\n");
    printf("1. Cargar usuarios\n");
    printf("2. Operaciones individuales\n");
    printf("3. Cargar operaciones\n");
    printf("4. Reporte estados de cuenta\n");
    printf("5. Salir\n");
    printf("-------------------------------\n");
    printf("Seleccione una opción: ");
}

int main() {

    struct User users[MAX_USERS];
    struct Error errors[MAX_USERS];
    
    int num_users = 0;
    int num_errors = 0;
    int option;
    do {
        menu();
        if (scanf("%d", &option) != 1) {
            printf("Error: Entrada no válida. Intente de nuevo.\n");
            while (getchar() != '\n'); // Limpiar el búfer de entrada
            continue;
        }
        getchar();

        switch (option) {
            case 1:
                int users_loaded[NUM_THREADS];
                load_users(users, &num_users, errors, &num_errors, users_loaded);
                create_log_file(num_users, errors, num_errors, users_loaded);
                memset(errors, 0, sizeof(errors));
                num_errors = 0;
                printf("Archivo de registro creado: carga_log.txt\n");
                
                break;
            case 2: // INDIVIDUALES
                operaciones_individuales(users, num_users);
                break;
            case 3:
                int operations_loaded[NUM_THREADS_OP];
                load_operations(users, &num_users, errors, &num_errors, operations_loaded);
                create_log_file_op(num_users, errors, num_errors, operations_loaded);
                memset(errors, 0, sizeof(errors));
                num_errors = 0;
                printf("Archivo de registro creado: carga_log.txt\n");
                break;
            case 4: // INDIVIDUALES
                create_csv(users, num_users);
                break;
            case 5: // INDIVIDUALES
                printf("Saliendo del programa.\n");
                break;
            default:
                printf("Opción no válida. Intente de nuevo.\n");
                break;
        }
        printf("Presiona Enter para continuar...");
        getchar(); // Espera a que el usuario presione Enter

        system("clear"); // Limpia la pantalla (para sistemas Unix/Linux)
        
    } while (option != 5);

    
    // for (int i = 0; i < num_users; i++) {
    //     printf("No. Cuenta %d, Nombre %s, Saldo %f \n", users[i].no_cuenta, users[i].nombre, users[i].saldo);
    // }

    return EXIT_SUCCESS;
}
