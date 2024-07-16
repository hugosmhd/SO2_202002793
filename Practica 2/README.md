Hugo Sebastian Martínez Hernández  
202002793  

# Practica 2 - Sistemas operativos 2
## En que consiste
Se realizo un programa en C que realiza operaciones bancarias básicas, como la carga de usuarios desde un archivo CSV, operaciones individuales como depósitos, retiros, transferencias y consultas de saldo, así como la carga de operaciones desde otro archivo CSV. El programa utiliza múltiples hilos para procesar la carga de usuarios y operaciones de manera más eficiente./stackedit.io/).

## Teoria
 
El multithreading, también conocido como multihilos o simplemente hilos, es una técnica de programación que permite que un proceso pueda realizar múltiples tareas de forma concurrente. Un proceso puede tener uno o más hilos de ejecución, cada uno de los cuales representa una unidad de ejecución independiente dentro del proceso. Estos hilos comparten recursos como la memoria y los archivos abiertos, pero tienen su propio estado de ejecución, incluyendo el contador de programa, la pila de llamadas y el registro de estado.

Los hilos son útiles para aprovechar los sistemas con múltiples núcleos de CPU, ya que permiten distribuir la carga de trabajo entre los núcleos y ejecutar tareas en paralelo, lo que puede mejorar significativamente el rendimiento de una aplicación.

Los semáforos son una herramienta de sincronización utilizada en programación concurrente para controlar el acceso a recursos compartidos, como variables o secciones críticas de código, entre múltiples hilos. Un semáforo esencialmente actúa como un contador que se utiliza para permitir o bloquear el acceso a un recurso compartido.

Hay dos tipos principales de semáforos:

1.  **Semáforos binarios**: Estos semáforos pueden tener dos valores: 0 y 1. Se utilizan típicamente para controlar el acceso a un recurso compartido donde solo puede haber un hilo accediendo al recurso a la vez. Cuando un hilo adquiere el semáforo, decrementa su valor a 0, bloqueando así a otros hilos que intenten adquirir el semáforo. Cuando el hilo que posee el semáforo lo libera, incrementa su valor a 1, permitiendo que otro hilo lo adquiera.
    
2.  **Semáforos contadores**: Estos semáforos pueden tener valores mayores que 1 y se utilizan para permitir el acceso simultáneo a un recurso compartido por múltiples hilos. Cada vez que un hilo adquiere el semáforo, su valor se decrementa. Cuando el hilo termina de usar el recurso, libera el semáforo, lo que incrementa su valor. Si el valor del semáforo es cero, los hilos que intenten adquirirlo se bloquearán hasta que otro hilo lo libere.
    

Los semáforos son una herramienta fundamental en la programación multihilo para prevenir condiciones de carrera, donde varios hilos intentan modificar un recurso compartido al mismo tiempo, lo que puede llevar a resultados impredecibles o incorrectos. Al utilizar semáforos correctamente, los programadores pueden coordinar eficazmente la ejecución de múltiples hilos y garantizar la consistencia y la integridad de los datos compartidos.
  
El código proporcionado es un programa en C que realiza operaciones bancarias básicas, como la carga de usuarios desde un archivo CSV, operaciones individuales como depósitos, retiros, transferencias y consultas de saldo, así como la carga de operaciones desde otro archivo CSV. El programa utiliza múltiples hilos para procesar la carga de usuarios y operaciones de manera más eficiente.

A continuación, se proporciona una explicación detallada del código:

### Librerías Utilizadas:

-   `stdio.h`: Proporciona funciones para entrada y salida estándar.
-   `stdlib.h`: Proporciona funciones estándar de utilidad, como la gestión de memoria dinámica, control de procesos y otras.
-   `pthread.h`: Proporciona funciones para la programación de hilos (threads).
-   `semaphore.h`: Define las funciones y macros necesarias para trabajar con semáforos.
-   `string.h`: Proporciona funciones para manipular cadenas de caracteres.

### Estructuras:

-   `User`: Estructura para almacenar información de usuario, incluyendo número de cuenta, nombre y saldo.
-   `Error`: Estructura para almacenar información sobre errores encontrados durante la carga de usuarios u operaciones.
-   `ThreadData`: Estructura para pasar datos a los hilos. Contiene información como el archivo a leer, identificador de hilo, datos de usuarios, datos de errores, etc.

### Variables Globales:

-   `pthread_mutex_t mutex`: Mutex utilizado para sincronizar el acceso a secciones críticas relacionadas con la carga de usuarios.
-   `pthread_mutex_t mutex2`: Mutex utilizado para sincronizar el acceso a secciones críticas relacionadas con la carga de operaciones.
-   `int deposit_count`, `int withdrawal_count`, `int transfer_count`: Contadores globales para el número total de operaciones de depósito, retiro y transferencia, respectivamente.

### Funciones Principales:

1.  `load_data_users`: Función ejecutada por cada hilo para cargar datos de usuarios desde un archivo CSV.
2.  `load_users`: Función principal para cargar usuarios desde un archivo CSV utilizando múltiples hilos.
3.  `create_log_file`: Función para crear un archivo de registro que contiene detalles sobre la carga de usuarios y posibles errores.
4.  `load_data_operations`: Función ejecutada por cada hilo para cargar operaciones desde un archivo CSV.
5.  `load_operations`: Función principal para cargar operaciones desde un archivo CSV utilizando múltiples hilos.
6.  `create_log_file_op`: Función para crear un archivo de registro que contiene detalles sobre las operaciones realizadas y posibles errores.
7.  `create_csv`: Función para crear un archivo CSV que contiene los estados de cuenta de los usuarios después de realizar las operaciones.
8.  `retiro`, `deposito`, `transferencia`, `consultar`: Funciones para realizar operaciones individuales (retiro, depósito, transferencia y consulta de saldo).
9.  `operaciones_individuales`: Función para el menú de operaciones individuales.
10.  `menu`: Función para el menú principal del programa.
11.  `main`: Función principal del programa que controla el flujo del mismo.