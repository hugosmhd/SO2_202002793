
# Practica 1 Laboratorio de Sistemas Operativos 2

**Estudiante**: Hugo Sebastian Martínez Hernández

**Carné**: 202002793

**Auxiliar**: Derek Esquivel

**Sección**: A


# Explicación de creación de procesos hijos con el comando fork()

Un "fork" es una función que crea un nuevo proceso, que es una copia exacta del proceso que lo llama (proceso padre). Después de que se llama a `fork()`, tanto el proceso padre como el hijo continúan ejecutándose desde el punto en que se realizó la llamada a `fork()`, pero con diferentes identificadores de proceso (PID).

El proceso hijo recibe un PID de 0, mientras que el proceso padre recibe el PID del hijo. El nuevo proceso (hijo) tiene su propio espacio de memoria, pero inicialmente comparte todos los mismos recursos del proceso padre, incluidos los archivos abiertos, los descriptores de archivos, los semáforos, las señales y otros recursos del sistema.
```c
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
```

En el proceso padre, devuelve el PID del hijo recién creado, mientras que en el proceso hijo, devuelve 0. Si hay un error al crear el nuevo proceso, `fork()` devuelve -1.

Para esta practica se pidio crear 2 hijos con el comando fork(), por lo que se decidio hacerlo por medio de un for. Además los hijos ejecutan código desde un archivo .bin que se encarga de ejecutar su tarea como se explicara más adelante.


## Explicacion del código de los procesos hijos

```c
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
```

En primer lugar se hace verificacion que vengan los parámetros desde donde se ejecuta el hijo, estos parametros nos serviran de apoyo para poder hacer el proceso de contadores para el reporte final. Si no se proporciona la dirección de memoria compartida, muestra un mensaje de uso y sale del programa con un código de fallo.

Se abre un archivo en modo de adición y lectura/escritura (`O_APPEND | O_RDWR`) utilizando la función `open()`. Luego de abrir el archivo entra en un bucle infinito para realizar las llamadas aleatorias 

Se creo una funcion llamada random_calls:
```c
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
```
Se genera un número aleatorio entre 0 y 2 (inclusive) utilizando la función `rand() % 3`. Este número representa la operación aleatoria que se realizará en el archivo: 0 para escribir (`write`), 1 para leer (`read`), y 2 para buscar (`lseek`).

Se declara un buffer de caracteres llamado `buffer` con una longitud de `LINE_LENGTH + 1`, donde `LINE_LENGTH` es una constante que determina el tamaño del buffer. Este buffer se utilizará para almacenar datos para escribir o leer en el archivo.

Operaciones realizadas:
-   Si la operación aleatoria es 0, se llena el buffer con caracteres aleatorios ('A' a 'Z') y se escribe en el archivo utilizando la función `write()`. Luego se incrementa el contador de llamadas de escritura (`counter->write_calls`).
-   Si la operación aleatoria es 1, se lee una línea del archivo en el buffer utilizando la función `read()`. Luego se incrementa el contador de llamadas de lectura (`counter->read_calls`).
-   Si la operación aleatoria es 2, se realiza una búsqueda al inicio del archivo utilizando la función `lseek()`. Luego se incrementa el contador de llamadas de búsqueda (`counter->seek_calls`).

Por último, se incrementa el contador total de llamadas del sistema (`counter->total_syscalls`) independientemente de la operación realizada.

## Manejo de contadores

Para el manejo de los contadores se realizo con memoria compartida eso con la finalidad de que los dos procesos hijos fueran capaces de actualizar los contadores y además el proceso padre tuviera acceso a los mismos al terminar el programa, el código siguiente se agrego en el archivo del padre.
```c
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
```
- Se utiliza la función `shmget()` para obtener el identificador de la memoria compartida. Se usa `IPC_PRIVATE` como el identificador de clave, que indica al sistema que debe crear una nueva región de memoria compartida. `sizeof(Counter)` especifica el tamaño de la memoria compartida a asignar, que es del tamaño de la estructura `Counter`. `IPC_CREAT | 0666` indica que se debe crear la memoria compartida si no existe y se establecen permisos de lectura y escritura.

- Se utiliza la función `shmat()` para adjuntar la memoria compartida al espacio de direcciones del proceso. Se pasa el identificador de la memoria compartida `shmid`, un puntero nulo para que el sistema elija automáticamente una dirección de memoria y se especifica `0` para las banderas de adjuntar la memoria.
- Una vez adjuntada la memoria compartida con éxito, se accede a ella a través del puntero `counter` y se inicializan los contadores `read_calls`, `write_calls` y `seek_calls` a cero.

## SystemTap

SystemTap (stap), que es una herramienta de monitoreo y diagnóstico para sistemas basados en Linux. El siguiente script está diseñado para interceptar y monitorear llamadas al sistema relacionadas con las operaciones de lectura (`syscall.read`), escritura (`syscall.write`) y búsqueda (`syscall.lseek`)

```bash
#!/usr/bin/stap

probe syscall.read {
    if(pid() == $1 || pid() == $2){
        ts = gettimeofday_ns();
        printf("Proceso %d: %s %s\n",pid(), name, ctime(ts / 1000000000));
    }
}

probe syscall.write {
    if(pid() == $1 || pid() == $2){
        ts = gettimeofday_ns();
        printf("Proceso %d: %s %s\n",pid(), name, ctime(ts / 1000000000));
    }
}

probe syscall.lseek {
    if(pid() == $1 || pid() == $2){
        ts = gettimeofday_ns();
        printf("Proceso %d: %s %s\n",pid(), name, ctime(ts / 1000000000));
    }
}
```
-   El script comienza con `#!/usr/bin/stap`, que indica que este es un script SystemTap y especifica el intérprete (`stap`) que debe usar para ejecutarlo.
-   Luego, hay tres bloques `probe` que corresponden a las tres llamadas al sistema que se están monitoreando: `syscall.read`, `syscall.write`, y `syscall.lseek`.
-  La función `pid()` devuelve el PID del proceso que realizó la llamada al sistema.
-   Los argumentos de línea de comandos `$1` y `$2` se utilizan para especificar dos PID en los que se está interesado en monitorear.
