# CONTEXTO DEL PROYECTO: Codexion

## 1. Visión General
Proyecto basado en el problema clásico de la concurrencia (estilo Philosophers de 42). Un grupo de programadores (hilos) comparten *dongles* (recursos limitados) para poder compilar.
- **Estados:** Compiling (necesita 2 dongles compartidos), Debugging (no necesita recursos), Refactoring (pensando, esperando recursos).
- **Planificación (Schedulers):** 
  - `fifo` (0): Orden de llegada estricto.
  - `edf` (1): Earliest Deadline First (prioridad al que esté más cerca del *burnout*).

## 2. Arquitectura de Datos

### `t_env` (Estado Global de la Simulación)
Contiene los parámetros, la cola de espera, y los mutexes globales.
- `arbitrator_mutex`: El mutex central. Protege la asignación de *dongles*, la manipulación de la cola de espera (`enqueue`/`dequeue`) y las variables de condición (`pthread_cond_wait`/`broadcast`).
- `write_mutex`: Protege los `printf` para que las trazas de log no se pisen entre sí.
- `end_mutex`: Protege exclusivamente la bandera de estado `simulation_end`.

### `t_coder` (Estado Individual por Hilo)
Representa a cada programador.
- `state_mutex`: Mutex individual que protege `last_compile_start` y `compiles_done`.

## 3. Prevención de Deadlocks y Lock-Order Inversion
Para satisfacer las estrictas reglas de ThreadSanitizer y Helgrind, se ha establecido este diseño de bloqueos:
- **Watcher:** Cuando verifica a un Coder, bloquea `state_mutex`, copia las variables a memoria local, y **desbloquea inmediatamente `state_mutex`** ANTES de evaluar si el coder ha muerto. Si muere, llama a `burnout_stop` (que bloquea `arbitrator_mutex`). 
- **Regla de oro:** JAMÁS bloquear `arbitrator_mutex` mientras se sostiene un `state_mutex` (o viceversa), ya que causaría una inversión del orden de bloqueos con el flujo natural del Coder.

## 4. Lost Wakeup Prevention (Variable de Condición)
El *broadcast* (`pthread_cond_broadcast`) se realiza **siempre con el `arbitrator_mutex` bloqueado**. Esto elimina la "ventana ciega" donde un Coder verifica si puede tomar un dongle, se le quita la CPU, el Watcher emite la señal de fin, y el Coder se duerme para siempre ignorando el final de la simulación.

## 5. Batería de Pruebas (Makefile)
El proyecto cuenta con un sistema robusto de testing dividido en dos objetivos principales:
- `make termtest`: Ejecuta pruebas de argumentos inválidos y las 4 simulaciones core en vivo por terminal.
- `make logtest`: Ejecuta validaciones silenciosas y exporta los resultados a la carpeta `logs/`.
  - **Pruebas de error de parseo** (`logs/error_tests.log`).
  - **Matriz de Helgrind** (`logs/helgrind.log`): Valida bloqueos y orden POSIX.
  - **Matriz de Memcheck** (`logs/memcheck.log`): Valida leaks (`malloc`/`free`) y origen de variables.
  - **Matriz de ThreadSanitizer** (`logs/tsan.log`): Recompila con `-fsanitize=thread` y valida *data races*.

### Matriz 2x2 Core (Las 4 pruebas de fuego)
1. `FIFO` + *Burnout* (Ej: `./codexion 5 800 200 200 200 100 50 fifo`)
2. `FIFO` + *Success* (Ej: `./codexion 4 1000 200 200 200 7 50 fifo`)
3. `EDF` + *Burnout* (Ej: `./codexion 5 800 200 200 200 100 50 edf`)
4. `EDF` + *Success* (Ej: `./codexion 4 1000 200 200 200 7 50 edf`)