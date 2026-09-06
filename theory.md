
# Anatomía de un Hilo (Thread) vs. Proceso
Cuando ejecutas tu programa ./codexion, el sistema operativo (SO) crea un Proceso. Este proceso tiene un espacio de memoria virtual propio (el Memory Frame) dividido en segmentos:  
- **Text (Código):** Las instrucciones binarias compiladas.
- **Data & BSS:** Variables globales y estáticas.
- **Heap:** Memoria dinámica (malloc).
- **File Descriptors:** Archivos abiertos, sockets, etc.
- **Stack (Pila):** Variables locales, argumentos de funciones y el historial de llamadas a funciones.

## ¿Qué hace pthread_create?
En Linux, la librería pthreads por debajo utiliza una llamada al sistema (syscall) llamada `clone()`. A diferencia de ``fork()`` (que copia toda la casa para crear un proceso hijo independiente), ``clone()`` crea un Hilo.

### ¿Qué comparten los hilos?
Comparten la casa entera: el ``Text``, el ``Data``, los ``File Descriptors`` y, lo más importante, el ``Heap``. Por eso tus 250 programadores pueden ver y tocar la variable ``env->simulation_end``.  
### ¿Qué NO comparten (qué es privado de cada hilo)?  
- Su propio ``Stack``: Cada hilo tiene su propia pila de ejecución (normalmente de ``8 MB``). Las variables locales que declaras dentro de ``coder_routine`` existen solo en la pila de ese hilo.
- ``Registros de la CPU``: Cada hilo tiene su propia copia virtual del Program Counter (PC) y el Stack Pointer (SP).
- ``Thread ID (TID)``: Su identificador único.

## El Gestor de Procesos (Scheduler) de Linux

En Linux, hilos y procesos son vistos internamente casi como lo mismo: se llaman Tareas (representadas por la estructura task_struct en el kernel). El planificador actual de Linux (históricamente el Completely Fair Scheduler - CFS, ahora evolucionando a EEVDF) trocea el tiempo de la CPU en pequeñas rebanadas (time slices, de unos pocos milisegundos).
Cuando el time slice de tu hilo termina, o cuando el hilo hace una operación bloqueante (como tu ``pthread_cond_wait``), el Scheduler interviene de forma transparente.

## El Cambio de Contexto (Context Switch)

Cuando la CPU deja de ejecutar el Hilo A para ejecutar el Hilo B, ocurre un Context Switch. Es una operación costosa.

### ¿Cómo funciona y qué se guarda?:
1. **Interrupción:** Un temporizador de hardware o una syscall (como el mutex) lanza una interrupción. La CPU entra en "Modo Kernel" (Modo Dios).
2. **Guardado de estado:** El Kernel coge los valores actuales de los Registros Físicos de la CPU:
- ``Program Counter (PC)``: ¿En qué línea exacta de código estábamos?
- ``Stack Pointer (SP)``: ¿Por dónde va nuestra pila local?
- ``Registros de propósito general (RAX, RBX...)``: Los cálculos a medias. Este estado se guarda en la memoria principal (dentro del task_struct del Hilo A).
3. **Carga de estado:** El Kernel lee el task_struct del Hilo B y vuelca sus valores guardados de vuelta a los registros físicos de la CPU.
4. **Reanudación:** La CPU vuelve a "Modo Usuario" y el Hilo B continúa exactamente donde lo dejó.

### El drama de las Cachés (L1, L2, L3 y TLB)
El mayor coste de un Context Switch no es guardar los registros, es la basura térmica.  

Mientras el Hilo A se ejecutaba, llenó la velocísima memoria caché ``L1`` de la CPU con sus datos.
Cuando entra el Hilo B, los datos de A no le sirven. Al empezar a pedir sus propios datos a la RAM, sobrescribe la caché ``L1``.
Cuando el Hilo A vuelva a ejecutarse más tarde, su caché estará fría (``Cache Miss``), y tendrá que esperar ciclos de CPU larguísimos para volver a traer sus datos de la RAM principal. 

Por qué importa en el proyecto: Hacer esperas activas (while (1) try_lock) o despertar hilos inútilmente (broadcast mal gestionado) provoca ráfagas de miles de Context Switches por segundo. Tu CPU gasta más tiempo cambiando de contexto que ejecutando tu código real, colapsando el sistema.

# Peligros de la Concurrencia
### Data Races (Condición de Carrera)
Ocurre cuando dos hilos acceden a la misma variable compartida en memoria al mismo tiempo, y al menos uno de ellos la está modificando (escribiendo).
El mito de la atomicidad: Podrías pensar que ``coder->compiles_done++`` es seguro porque es una sola línea de código en C. Falso. En lenguaje ensamblador, esa línea se divide en tres instrucciones de hardware:
1. ``READ``: Carga el valor de la RAM a un registro de la CPU (ej. valor 5).
2. ``ADD``: Suma 1 al registro (valor 6).
3. ``WRITE``: Escribe el 6 de vuelta en la RAM.

Si el Hilo 1 y el Hilo 2 leen a la vez (ambos leen 5), ambos suman 1 (tienen 6), y ambos escriben 6. En lugar de tener 7, has perdido una compilación. Por eso TODO lo que se lea/escriba por múltiples hilos debe estar envuelto en un Mutex. El Mutex garantiza la Atomicidad (la operación ocurre entera, sin interrupciones).

### Deadlocks (Abrazo Mortal)
Ocurre cuando dos o más hilos se quedan esperando mutuamente de forma infinita. Para que un Deadlock ocurra, deben darse matemáticamente las 4 Condiciones de Coffman simultáneamente:
1. **Exclusión Mutua**: El recurso no se puede compartir (tus dongles/mutex).
2. **Retención y Espera (Hold and Wait)**: Un hilo tiene un recurso agarrado mientras pide otro. (El programador coge el dongle izquierdo y espera por el derecho).
3. **No apropiación (No Preemption)**: El SO no puede arrancarle el mutex de las manos al programador por la fuerza; el programador debe soltarlo voluntariamente.
4. **Espera Circular**: El programador 0 espera al 1, el 1 al 2, y el 250 espera al 0.
Cómo rompe el proyecto el Deadlock: Los programadores nunca bloquean un dongle individualmente mientras piden el otro. Le piden ambos al Árbitro de forma atómica. Si no están los dos, se van a dormir sin retener nada.

### Otros Conceptos de Nivel Experto
**Inanición (Starvation):** No confundir con Deadlock. En el Deadlock nadie avanza. En la inanición, el sistema funciona de maravilla, pero un hilo concreto jamás consigue recursos porque siempre se le adelantan hilos "más fuertes" o más rápidos. La cola EDF/FIFO soluciona exactamente esto, garantizando que el más antiguo o el más crítico siempre sea atendido.

**Inversión de Prioridades (Priority Inversion):** Un hilo de baja prioridad agarra un Mutex. De repente, un hilo de altísima prioridad necesita ese Mutex. El sistema colapsa porque el hilo de alta prioridad se queda bloqueado esperando a un hilo de baja prioridad, el cual, para colmo, apenas recibe tiempo de CPU para terminar su tarea y soltar la llave.


# Funciones POSIX
## Gestión de Hilos (Creación y Destrucción)
### ``pthread_create``
Le dice al sistema operativo que cree un nuevo hilo de ejecución que correrá en paralelo al programa principal.

**Argumentos:**
1. ``pthread_t *thread``: Un puntero a la variable donde se guardará el "DNI" o identificador del hilo (en tu caso, tu array env->coder_threads[i] o env->watcher_thread). 
2. ``const pthread_attr_t *attr``: Atributos especiales para el hilo (tamaño de pila, prioridades). En 42 siempre se manda NULL para usar los valores por defecto.
3. ``void *(*start_routine) (void *)``: Un puntero a la función que el hilo va a ejecutar nada más nacer (ej. coder_routine o watcher_routine). 
4. ``void *arg``: El argumento que le pasas a esa función (le pasamos &env->coders[i] o env). 


### ``pthread_join``
Hace que el hilo que llama a esta función (el main) se detenga por completo hasta que el hilo especificado termine su función y muera. Además, recoge la basura/memoria residual que deja el hilo al morir.

**Argumentos:**
1. ``pthread_t thread``: El "DNI" del hilo al que quieres esperar.
2. ``void **retval``: Un puntero para atrapar lo que sea que devuelva el hilo con su return. Como tus rutinas devuelven NULL, siempre pasamos NULL aquí. 

## Los Mutexes (Candados)
Un mutex (Mutual Exclusion) es literalmente una llave única. Si un hilo la tiene, los demás tienen que hacer cola en la puerta hasta que la suelte.

### ``pthread_mutex_init``
Prepara la variable de tipo pthread_mutex_t para ser usada.

**Argumentos:**
1. ``pthread_mutex_t *mutex``: Puntero al mutex que vas a inicializar.
2. ``const pthread_mutexattr_t *attr``: Atributos especiales. De nuevo, siempre NULL.

### ``pthread_mutex_lock``
Intenta echar la llave. Si el candado está abierto, lo cierra y el hilo continúa ejecutando código. Si el candado ya estaba cerrado por otro hilo, el hilo actual se congela en esta línea hasta que el otro lo abra.

**Argumentos:**
``pthread_mutex_t *mutex``: el candado a bloquear. 

### ``pthread_mutex_unlock``
Abre el candado. Si había otros hilos congelados en el lock esperando por esta llave, el sistema operativo despertará a uno de ellos para dársela.

**Argumentos:** 
``pthread_mutex_t *mutex``: el candado a desbloquear. 

### ``pthread_mutex_destroy``
Destruye el candado y libera la memoria interna que el sistema operativo asignó para gestionarlo. No se puede destruir un candado que está actualmente bloqueado.

**Argumentos:** ``pthread_mutex_t *mutex``.

## Variables de Condición (Control de Sueño)
Las variables de condición se usan cuando un hilo necesita esperar a que pase algo específico (ej. "que mis dongles estén libres") sin tener que estar preguntando constantemente y quemando la CPU (polling).

### ``pthread_cond_init`` y ``pthread_cond_destroy``
Para qué sirven: Igual que con los mutexes, inicializan y destruyen las variables de tipo ``pthread_cond_t``. 

**Argumentos:** El puntero a la variable y NULL para los atributos.

### ``pthread_cond_wait``
Pone a dormir al hilo a nivel de sistema operativo y desbloquea el mutex asociado al mismo tiempo (de forma atómica). Al despertar, vuelve a bloquear el mutex automáticamente.

**Argumentos:**
1. ``pthread_cond_t *cond``: La "cama" en la que se va a dormir.
2. ``pthread_mutex_t *mutex``: El candado que tiene actualmente bloqueado y que debe soltar antes de dormir (en tu caso, el arbitrator_mutex).


### ``pthread_cond_timedwait``
Es igual que el wait, pero le pasas una fecha/hora de caducidad. Si nadie lo despierta antes de esa hora, se despierta solo y devuelve un error de timeout.

**Argumentos:**

El cond, el mutex, y un ``struct timespec *abstime`` con la hora exacta en la que debe sonar su alarma.

Aunque se puede usar para simular la espera de muerte, nosotros estamos usando nuestra función propia codex_usleep con un bucle de micro-comprobaciones, lo cual es mucho más controlable en 42.

### ``pthread_cond_broadcast`` (y ``pthread_cond_signal``)
Son el despertador. signal despierta a un solo hilo que esté dormido en esa variable de condición. broadcast despierta a absolutamente todos los hilos que estén durmiendo ahí.

**Argumentos:** ``pthread_cond_t *cond``.

### Detalles sobre el uso de POSIX ``pthread_cond``
- **Por qué pthread_cond_wait está dentro de un while y no de un if?**:  
Esta es una pregunta clásica de evaluación. En POSIX, existe un fenómeno llamado ``Spurious Wakeup`` (Despertar espurio). A veces, el sistema operativo despierta a un hilo dormido por accidente (por una señal de red, un evento de hardware, etc.) aunque nadie le haya hecho broadcast. Si usáramos un if, el hilo despertaría por accidente, ignoraría el chequeo y robaría los dongles. Con un while, si despierta por error, la función can_take_dongles dirá: "Oye, que no es tu turno", y el hilo se volverá a dormir de inmediato. **Esto es obligatorio en C.**

- Aunque la especificación de POSIX no te prohíbe técnicamente llamar a pthread_cond_broadcast sin el mutex bloqueado, hacerlo sin él introduce una condición de carrera crítica conocida como ``Lost Wakeup`` (Despertar Perdido).
El escenario de fallo:
	1. **CODER** Revisa can_take_dongles() -> FALSO.
	2. **CODER** La CPU le quita el turno JUSTO ANTES de ejecutarse pthread_cond_wait.
	3. **WATCHER** Pone simulation_end = 1.
	4. **WATCHER** Emite pthread_cond_broadcast().¡Como el Coder aún no está en wait, la señal se pierde en el aire!
	5. **CODER** El Coder retoma la CPU y ejecuta pthread_cond_wait(...).
	6. **CODER:** EL CODER SE QUEDA DORMIDO PARA SIEMPRE.

Cómo lo soluciona bloquear el arbitrator_mutex:  
	Cuando el Watcher hace ``pthread_mutex_lock(&env->arbitrator_mutex)`` antes de hacer el broadcast, se elimina esa ventana ciega:
	1. **Si el Coder llegó primero:** Ya está de verdad dentro de pthread_cond_wait (lo que significa que ya liberó el mutex y está escuchando la variable de condición). Recibirá la señal del Watcher de forma 100% garantizada.
	2. Si el Watcher llegó primero: Al tener el mutex agarrado, el Coder no podrá evaluar el while ni intentar dormirse hasta que el Watcher termine, cambie el estado y libere el mutex. Cuando el Coder por fin adquiera el mutex, evaluará simulation_end == 1 y ni siquiera intentará dormirse.

En resumen:
	El mutex de la variable de condición no protege a la propia llamada pthread_cond_broadcast; protege el estado que determina si el hilo debe dormirse o no (simulation_end / can_take_dongles).
	Bloquear arbitrator_mutex en el Watcher garantiza que la modificación del estado y el aviso de despertar ocurran como una única operación atómica para todos los programadores.

# Diseño
## ``t_coder``

- ``id``: Su número de identificación (de 0 a 249).
- ``compiles_done``: Un contador de cuántas veces ha logrado compilar.
- ``last_compile_start``: La marca de tiempo (en milisegundos) exacta en la que empezó su última compilación.
- ``state_mutex``: (Decisión de diseño clave). Es un candado personal. Como el Watcher está constantemente leyendo ``compiles_done`` y ``last_compile_start`` para ver si el coder ha muerto o ha terminado, y el propio coder está escribiendo en esas variables, necesitamos este mutex para evitar una Data Race.
- ``env``: Un puntero a la estructura global.

## ``t_env``

### Control de Estado y Tiempo

- ``start_time``: El milisegundo exacto en el que arranca la simulación. Todas las impresiones por pantalla se calculan restando el tiempo actual a este valor para que los logs empiecen en 0.
- ``simulation_end``: Un interruptor (0 o 1). Si alguien muere o todos terminan, se pone a 1. Es la señal para que todos los hilos rompan sus bucles infinitos y terminen de forma limpia.
- ``end_mutex``: El candado que protege a simulation_end. Como todos los coders lo leen constantemente en su while (1) y el Watcher lo puede modificar en cualquier momento, debe estar protegido.

### Sincronización de Salida
- ``write_mutex``: Evita que dos hilos escriban en la consola al mismo tiempo. Sin esto, un mensaje como 200 1 is compiling y 200 2 is debugging podrían mezclarse y salir como 200 1 200 2 is is compiling debugging.

### El Árbitro Central

- ``heap_dongles[250]``: Representa el estado físico de los dongles en la mesa. Dependiendo de cómo lo implementemos, puede guardar quién tiene cada dongle o en qué milisegundo estará disponible (por el dongle_cooldown).

- ``queue[250]``: Es la sala de espera. Cuando un coder quiere dongles pero no están disponibles, el árbitro anota su id en esta cola. Si es modo FIFO, se anota al final; si es modo EDF, se inserta ordenado según su urgencia.
- ``queue_size``: Nos dice cuánta gente hay actualmente en la sala de espera.

arbitrator_mutex: (Decisión de diseño clave). Es la puerta de entrada al Árbitro. Ningún coder toca un dongle directamente. Primero bloquean este mutex, le dicen al árbitro lo que quieren, el árbitro manipula heap_dongles y queue, y luego sueltan el mutex. Esto serializa las peticiones y evita los Deadlocks.
cond_coders[250]: Variables de condición (una para cada coder). Si el coder llega al árbitro y sus dongles están ocupados, en lugar de quedarse consumiendo CPU (espera activa), el árbitro le dice: "Duérmete en tu cond_coder específico". Cuando sus dongles se liberen, el árbitro usará pthread_cond_signal para despertar única y exclusivamente a ese coder.
D. Memoria de Hilos
coders[250]: El array de estructuras individuales que definimos arriba. Almacena la información.
coder_threads[250]: Las variables del sistema (pthread_t) que manejan la ejecución real en los núcleos de tu procesador.
watcher_thread: El hilo de la parca que vigila desde las sombras.
Resumen de las Decisiones de Diseño
1. Estado descentralizado, control centralizado: Cada coder guarda su propia "salud" (state_mutex), pero la repartición de recursos se hace en una sala central (arbitrator_mutex). Esto evita cuellos de botella gigantes, ya que un coder compilando no bloquea a los demás.
2. Uso de Variables de Condición en lugar de Polling: En lugar de hacer que un coder pregunte cada milisegundo "¿Están mis dongles listos?" (lo que freiría la CPU), usamos cond_coders. El hilo se pausa a nivel de sistema operativo y no consume recursos hasta que lo despiertan.
3. La separación queue / heap_dongles: Permite que el sistema sea modular. El árbitro primero mira la queue para ver quién tiene prioridad (aplicando FIFO o EDF), y luego mira heap_dongles para ver si se le puede dar el recurso. Es la base de un Scheduler profesional.

El fantasma de pthread_mutex_trylock
Has tocado la razón de ser de toda nuestra arquitectura. La prohibición de trylock en 42 es intencionada: te obligan a aprender sincronización avanzada en lugar de usar la fuerza bruta.
¿Qué hace trylock?
A diferencia de lock (que te congela si el candado está cerrado), trylock intenta abrirlo; si está cerrado, no se bloquea, sino que te devuelve un error al instante (como diciendo: "Ocupado, vuelve luego").
?￯ﾸﾏ Elementos de tu t_env que existen POR CULPA de no tener trylock:
1. pthread_mutex_t arbitrator_mutex
2. pthread_cond_t cond_coders[250]
3. int queue[250] y int queue_size
4. int heap_dongles[250]
Si no tenemos trylock y un hilo hace pthread_mutex_lock(&dongle_izquierdo) y luego pthread_mutex_lock(&dongle_derecho), corre el riesgo de quedarse congelado con el izquierdo en la mano. Si su vecino hace lo mismo, se produce el temido Deadlock. Para evitarlo sin trylock, tuvimos que crear la figura del Árbitro (una sala central donde preguntas por los dos dongles a la vez) y las Variables de Condición (para dormir si no están listos).
? ¿Cómo sería el proyecto SI tuviéramos trylock?
El diseño sería radicalmente distinto (y mucho más primitivo). Desaparecería el Árbitro, las colas y las variables de condición. Tu t_env solo tendría un array de mutexes: pthread_mutex_t dongles[250].
La rutina de coger los recursos en tu coder_routine se vería más o menos así:
C
// Bucle infinito hasta conseguir ambos dongles (Spin-lock)
while (1)
{
    // Intenta coger el izquierdo (no bloqueante)
    if (pthread_mutex_trylock(&env->dongles[id_izquierdo]) == 0)
    {
        // Si lo consigue, intenta coger el derecho (no bloqueante)
        if (pthread_mutex_trylock(&env->dongles[id_derecho]) == 0)
        {
            break ; // ¡Éxito! Tenemos los dos, salimos del bucle a compilar.
        }
        else
        {
            // Si el derecho está ocupado, SUELTA el izquierdo inmediatamente
            // para no bloquear al vecino (prevención de Deadlock)
            pthread_mutex_unlock(&env->dongles[id_izquierdo]);
        }
    }
    // Si falla, duerme un microsegundo y vuelve a intentarlo (Polling)
    usleep(500);
}
⚖️ ¿Cuál diseño es mejor en el mundo real?
Aunque el código con trylock parece más fácil de leer, tu diseño actual con el Árbitro y cond_coders es infinitamente superior por dos motivos: 
Evita freír la CPU: Con trylock, los hilos se quedan dando vueltas en un bucle ciego (while(1)) preguntando mil veces por segundo "¿ya? ¿ya? ¿ya?" (esto se llama Polling o Busy Waiting). Con tu diseño, el hilo hace cond_wait y se apaga a nivel de hardware; consumo de CPU = 0% hasta que lo despiertan.
Permite EDF / FIFO: Con trylock los recursos se asignan por pura ley del más fuerte (el hilo que pregunte en el microsegundo exacto en que se libera el dongle, se lo lleva). Es imposible hacer una cola de prioridades justa.
En resumen: la prohibición de 42 te ha forzado a diseñar un gestor de recursos de nivel empresarial.
¿Viendo por fin clara toda la base teórica y por qué tu arquitectura es tan sólida, quieres que empecemos a escribir la función take_dongles donde el Árbitro decide quién entra a compilar?

El fantasma de pthread_cond_signal 
El problema de broadcast (El "Thundering Herd")
La diferencia teórica es:
signal: Despierta a 1 hilo que esté durmiendo.
broadcast: Despierta a TODOS los hilos que estén durmiendo en esa variable de condición.
Si en nuestro t_env hubiéramos puesto una sola variable de condición global (ej. pthread_cond_t wait_queue;) donde durmieran los 250 programadores, usar broadcast sería un desastre. Al liberar un par de dongles, despertarías a los 250 programadores de golpe. Todos lucharían por el arbitrator_mutex, 249 verían que no les toca y se volverían a dormir. Freirías la CPU.
?￯ﾸﾏ Por qué tu diseño esquiva esta bala
Nosotros no tenemos una variable global, tenemos un array: pthread_cond_t cond_coders[250].
Cuando el Coder 4 no tiene recursos, el Árbitro lo manda a dormir exclusivamente a cond_coders[4]. ¿Cuántos hilos hay durmiendo en cond_coders[4]? Solamente uno: el Coder 4.
Por lo tanto, cuando el Árbitro decida que es el turno del Coder 4, ejecutará: pthread_cond_broadcast(&env->cond_coders[4]);
Como solo hay un hilo asociado a esa variable específica, el broadcast despertará a "todos" los hilos de esa variable... que es exactamente 1 hilo. En la práctica, con este diseño descentralizado de variables de condición, broadcast se comporta exactamente igual que un signal de francotirador.
? El único cambio real
El único cambio será sintáctico a la hora de picar el código. Donde yo en mi explicación teórica dije "el Árbitro usará signal", en tu código escribiremos "el Árbitro usará broadcast". La matemática y la lógica de la cola de prioridades quedan intactas.
Viendo cómo nuestro diseño (Array de conds + Árbitro) ha esquivado esta trampa del subject de forma tan elegante... ¿Nos metemos por fin de lleno en el código de take_dongles y release_dongles para darle vida a este Árbitro?



Usleep
El comando usleep(tiempo) del sistema operativo no significa "duerme exactamente X microsegundos". En realidad significa: "Cede el control de la CPU y no vuelvas a despertar a este hilo en al menos X microsegundos".
El problema es que cuando ese tiempo pasa, el sistema operativo (su planificador o scheduler) tiene que volver a darle un hueco en el procesador. Si el procesador está ocupado con otros hilos (¡y vas a tener 250!), puede tardar 2, 5 o 10 milisegundos extra en despertarlo. En una simulación estricta donde la muerte se mide al milisegundo, ese retraso es fatal.
Gettimeofday
Cómo funciona gettimeofday
Para usarla, necesitas declararle una estructura llamada struct timeval, que por dentro tiene dos variables:
tv_sec: Los segundos que han pasado desde el 1 de enero de 1970 (Epoch).
tv_usec: Los microsegundos sobrantes.
Como en tu simulación vas a medir todo en milisegundos, la misión de tu función get_current_time es llamar a gettimeofday, coger esos dos valores, hacer la conversión matemática y devolver un único número (size_t) en milisegundos.
gettimeofday guarda la hora en 'time'. El segundo parámetro (zona horaria) siempre es NULL. 

Watcher
El Watcher es un único hilo que entra en un bucle infinito while(1). En cada vuelta del bucle, itera sobre el array de los 250 programadores (coders) para comprobar dos condiciones exactas:
Condición A: La Muerte por Burnout (Fallo)
Calcula: tiempo_actual - coder[i].last_compile_start.
Si ese tiempo es mayor o igual a env->time_to_burnout:
1. Bloquea el end_mutex y cambia env->simulation_end = 1.
2. Bloquea el write_mutex e imprime el mensaje de muerte.
3. Rompe el bucle y termina el hilo.
Condición B: La Victoria (Límite de compilaciones)
Si en los argumentos se especificó un límite de compilaciones (num_compiles_required > 0):
Cuenta cuántos coders tienen coder[i].compiles_done >= num_compiles_required.
Si ese contador llega al total de programadores (env->num_coders):
1. Bloquea el end_mutex y cambia env->simulation_end = 1.
2. Rompe el bucle y termina el hilo.
Regla de oro del Watcher para evitar bloqueos: Para leer last_compile_start o compiles_done de un Coder, el Watcher debe bloquear el state_mutex de ese Coder específico, leer la variable, y soltarlo inmediatamente. No puede mantener candados bloqueados mientras revisa a los demás, o congelará la simulación.

Desfase para evitar embotellamiento al principio de coders
¡Vamos a por ello! Primero explicamos a fondo la magia del desfase (o staggering) y luego dejamos escritas y niqueladas las tres acciones de los coders.
⚡ 1. El Desfase (Staggering): Por qué salva tu simulación
Imagina que abres las puertas de una tienda con rebajas y hay 250 personas en la puerta. Si todos entran exactamente al mismo milisegundo por la misma puerta de acceso (el arbitrator_mutex), el resultado es un estampida humana:
Todos intentan bloquear el mismo candado a la vez.
El procesador sufre un pico brutal de cambio de contexto (context switching).
Se genera lo que en sistemas operativos se llama el problema de la "Jauría Hambrienta" (Thundering Herd Problem).
Además, en el clásico problema de los Filósofos / Codexion, si tienes por ejemplo 4 dongles y 4 programadores, y todos se lanzan a por su primer dongle en el $t = 0\text{ ms}$, todos cogerán un dongle y ninguno podrá coger el segundo, cayendo en un Deadlock (Bloqueo Mutuo) instantáneo.
? La solución del desfase
Hacer que los programadores con ID impar esperen apenas $10\text{ ms}$ antes de entrar al bucle provoca un efecto en cadena genial:
1. Los programadores pares ($0, 2, 4\dots$) entran primero, piden sus dongles sin ninguna competición y empiezan a compilar.
2. $10\text{ ms}$ después, los impares ($1, 3, 5\dots$) intentan pedir dongles. Para entonces, la mitad de la mesa ya está organizada y los recursos fluyen en turnos de manera natural.1. La Estructura t_coder (El individuo)
Esta estructura representa el estado interno y personal de cada hilo programador.
id: Su número de identificación (de 0 a 249). Sirve para saber quién es, qué dongles le corresponden matemáticamente, y en qué índice de los arrays globales debe mirar.
compiles_done: Un contador de cuántas veces ha logrado compilar.
last_compile_start: La marca de tiempo (en milisegundos) exacta en la que empezó su última compilación. Es su "barra de vida".
state_mutex: (Decisión de diseño clave). Es un candado personal. Como el Watcher (un hilo distinto) está constantemente leyendo compiles_done y last_compile_start para ver si el coder ha muerto o ha ganado, y el propio coder está escribiendo en esas variables, necesitamos este mutex para evitar una Data Race. Nadie lee ni escribe la salud del coder sin pedirle la llave primero.
*env: Un puntero a la estructura global. Como en 42 están prohibidas las variables globales, esta es la forma en la que un individuo puede interactuar con el entorno (mirar la hora, pedir permiso al árbitro o escribir en pantalla).
2. La Estructura t_env (El Entorno y el Árbitro)
Aquí es donde reside la magia del proyecto. Lo dividiremos en bloques lógicos:
A. Control de Estado y Tiempo
start_time: El milisegundo exacto en el que arranca la simulación. Todas las impresiones por pantalla se calculan restando el tiempo actual a este valor para que los logs empiecen en 0.
simulation_end: Un interruptor (0 o 1). Si alguien muere o todos terminan, se pone a 1. Es la señal para que todos los hilos rompan sus bucles infinitos y terminen de forma limpia.
end_mutex: El candado que protege a simulation_end. Como todos los coders lo leen constantemente en su while (1) y el Watcher lo puede modificar en cualquier momento, debe estar protegido.
B. Sincronización de Salida
write_mutex: Evita que dos hilos escriban en la consola al mismo tiempo. Sin esto, un mensaje como 200 1 is compiling y 200 2 is debugging podrían mezclarse y salir como 200 1 200 2 is is compiling debugging.
C. El Árbitro Central (El motor de los recursos)
Aquí está la diferencia entre un proyecto básico y una simulación de CPU real.
heap_dongles[250]: Representa el estado físico de los dongles en la mesa. Dependiendo de cómo lo implementemos, puede guardar quién tiene cada dongle o en qué milisegundo estará disponible (por el dongle_cooldown).
queue[250]: Es la sala de espera. Cuando un coder quiere dongles pero no están disponibles, el árbitro anota su id en esta cola. Si es modo FIFO, se anota al final; si es modo EDF, se inserta ordenado según su urgencia.
queue_size: Nos dice cuánta gente hay actualmente en la sala de espera.
arbitrator_mutex: (Decisión de diseño clave). Es la puerta de entrada al Árbitro. Ningún coder toca un dongle directamente. Primero bloquean este mutex, le dicen al árbitro lo que quieren, el árbitro manipula heap_dongles y queue, y luego sueltan el mutex. Esto serializa las peticiones y evita los Deadlocks.
cond_coders[250]: Variables de condición (una para cada coder). Si el coder llega al árbitro y sus dongles están ocupados, en lugar de quedarse consumiendo CPU (espera activa), el árbitro le dice: "Duérmete en tu cond_coder específico". Cuando sus dongles se liberen, el árbitro usará pthread_cond_signal para despertar única y exclusivamente a ese coder.
D. Memoria de Hilos
coders[250]: El array de estructuras individuales que definimos arriba. Almacena la información.
coder_threads[250]: Las variables del sistema (pthread_t) que manejan la ejecución real en los núcleos de tu procesador.
watcher_thread: El hilo de la parca que vigila desde las sombras.
Resumen de las Decisiones de Diseño
1. Estado descentralizado, control centralizado: Cada coder guarda su propia "salud" (state_mutex), pero la repartición de recursos se hace en una sala central (arbitrator_mutex). Esto evita cuellos de botella gigantes, ya que un coder compilando no bloquea a los demás.
2. Uso de Variables de Condición en lugar de Polling: En lugar de hacer que un coder pregunte cada milisegundo "¿Están mis dongles listos?" (lo que freiría la CPU), usamos cond_coders. El hilo se pausa a nivel de sistema operativo y no consume recursos hasta que lo despiertan.
3. La separación queue / heap_dongles: Permite que el sistema sea modular. El árbitro primero mira la queue para ver quién tiene prioridad (aplicando FIFO o EDF), y luego mira heap_dongles para ver si se le puede dar el recurso. Es la base de un Scheduler profesional.
3. 1. La Estructura t_coder (El individuo)
Esta estructura representa el estado interno y personal de cada hilo programador.
id: Su número de identificación (de 0 a 249). Sirve para saber quién es, qué dongles le corresponden matemáticamente, y en qué índice de los arrays globales debe mirar.
compiles_done: Un contador de cuántas veces ha logrado compilar.
last_compile_start: La marca de tiempo (en milisegundos) exacta en la que empezó su última compilación. Es su "barra de vida".
state_mutex: (Decisión de diseño clave). Es un candado personal. Como el Watcher (un hilo distinto) está constantemente leyendo compiles_done y last_compile_start para ver si el coder ha muerto o ha ganado, y el propio coder está escribiendo en esas variables, necesitamos este mutex para evitar una Data Race. Nadie lee ni escribe la salud del coder sin pedirle la llave primero.
*env: Un puntero a la estructura global. Como en 42 están prohibidas las variables globales, esta es la forma en la que un individuo puede interactuar con el entorno (mirar la hora, pedir permiso al árbitro o escribir en pantalla).
2. La Estructura t_env (El Entorno y el Árbitro)
Aquí es donde reside la magia del proyecto. Lo dividiremos en bloques lógicos:
A. Control de Estado y Tiempo
start_time: El milisegundo exacto en el que arranca la simulación. Todas las impresiones por pantalla se calculan restando el tiempo actual a este valor para que los logs empiecen en 0.
simulation_end: Un interruptor (0 o 1). Si alguien muere o todos terminan, se pone a 1. Es la señal para que todos los hilos rompan sus bucles infinitos y terminen de forma limpia.
end_mutex: El candado que protege a simulation_end. Como todos los coders lo leen constantemente en su while (1) y el Watcher lo puede modificar en cualquier momento, debe estar protegido.
B. Sincronización de Salida
write_mutex: Evita que dos hilos escriban en la consola al mismo tiempo. Sin esto, un mensaje como 200 1 is compiling y 200 2 is debugging podrían mezclarse y salir como 200 1 200 2 is is compiling debugging.
C. El Árbitro Central (El motor de los recursos)
Aquí está la diferencia entre un proyecto básico y una simulación de CPU real.
heap_dongles[250]: Representa el estado físico de los dongles en la mesa. Dependiendo de cómo lo implementemos, puede guardar quién tiene cada dongle o en qué milisegundo estará disponible (por el dongle_cooldown).
queue[250]: Es la sala de espera. Cuando un coder quiere dongles pero no están disponibles, el árbitro anota su id en esta cola. Si es modo FIFO, se anota al final; si es modo EDF, se inserta ordenado según su urgencia.
queue_size: Nos dice cuánta gente hay actualmente en la sala de espera.
arbitrator_mutex: (Decisión de diseño clave). Es la puerta de entrada al Árbitro. Ningún coder toca un dongle directamente. Primero bloquean este mutex, le dicen al árbitro lo que quieren, el árbitro manipula heap_dongles y queue, y luego sueltan el mutex. Esto serializa las peticiones y evita los Deadlocks.
cond_coders[250]: Variables de condición (una para cada coder). Si el coder llega al árbitro y sus dongles están ocupados, en lugar de quedarse consumiendo CPU (espera activa), el árbitro le dice: "Duérmete en tu cond_coder específico". Cuando sus dongles se liberen, el árbitro usará pthread_cond_signal para despertar única y exclusivamente a ese coder.
D. Memoria de Hilos
coders[250]: El array de estructuras individuales que definimos arriba. Almacena la información.
coder_threads[250]: Las variables del sistema (pthread_t) que manejan la ejecución real en los núcleos de tu procesador.
watcher_thread: El hilo de la parca que vigila desde las sombras.
Resumen de las Decisiones de Diseño
1. Estado descentralizado, control centralizado: Cada coder guarda su propia "salud" (state_mutex), pero la repartición de recursos se hace en una sala central (arbitrator_mutex). Esto evita cuellos de botella gigantes, ya que un coder compilando no bloquea a los demás.
2. Uso de Variables de Condición en lugar de Polling: En lugar de hacer que un coder pregunte cada milisegundo "¿Están mis dongles listos?" (lo que freiría la CPU), usamos cond_coders. El hilo se pausa a nivel de sistema operativo y no consume recursos hasta que lo despiertan.
3. La separación queue / heap_dongles: Permite que el sistema sea modular. El árbitro primero mira la queue para ver quién tiene prioridad (aplicando FIFO o EDF), y luego mira heap_dongles para ver si se le puede dar el recurso. Es la base de un Scheduler profesional.
4. 1. La Estructura t_coder (El individuo)
Esta estructura representa el estado interno y personal de cada hilo programador.
id: Su número de identificación (de 0 a 249). Sirve para saber quién es, qué dongles le corresponden matemáticamente, y en qué índice de los arrays globales debe mirar.
compiles_done: Un contador de cuántas veces ha logrado compilar.
last_compile_start: La marca de tiempo (en milisegundos) exacta en la que empezó su última compilación. Es su "barra de vida".
state_mutex: (Decisión de diseño clave). Es un candado personal. Como el Watcher (un hilo distinto) está constantemente leyendo compiles_done y last_compile_start para ver si el coder ha muerto o ha ganado, y el propio coder está escribiendo en esas variables, necesitamos este mutex para evitar una Data Race. Nadie lee ni escribe la salud del coder sin pedirle la llave primero.
*env: Un puntero a la estructura global. Como en 42 están prohibidas las variables globales, esta es la forma en la que un individuo puede interactuar con el entorno (mirar la hora, pedir permiso al árbitro o escribir en pantalla).
2. La Estructura t_env (El Entorno y el Árbitro)
Aquí es donde reside la magia del proyecto. Lo dividiremos en bloques lógicos:
A. Control de Estado y Tiempo
start_time: El milisegundo exacto en el que arranca la simulación. Todas las impresiones por pantalla se calculan restando el tiempo actual a este valor para que los logs empiecen en 0.
simulation_end: Un interruptor (0 o 1). Si alguien muere o todos terminan, se pone a 1. Es la señal para que todos los hilos rompan sus bucles infinitos y terminen de forma limpia.
end_mutex: El candado que protege a simulation_end. Como todos los coders lo leen constantemente en su while (1) y el Watcher lo puede modificar en cualquier momento, debe estar protegido.
B. Sincronización de Salida
write_mutex: Evita que dos hilos escriban en la consola al mismo tiempo. Sin esto, un mensaje como 200 1 is compiling y 200 2 is debugging podrían mezclarse y salir como 200 1 200 2 is is compiling debugging.
C. El Árbitro Central (El motor de los recursos)
Aquí está la diferencia entre un proyecto básico y una simulación de CPU real.
heap_dongles[250]: Representa el estado físico de los dongles en la mesa. Dependiendo de cómo lo implementemos, puede guardar quién tiene cada dongle o en qué milisegundo estará disponible (por el dongle_cooldown).
queue[250]: Es la sala de espera. Cuando un coder quiere dongles pero no están disponibles, el árbitro anota su id en esta cola. Si es modo FIFO, se anota al final; si es modo EDF, se inserta ordenado según su urgencia.
queue_size: Nos dice cuánta gente hay actualmente en la sala de espera.
arbitrator_mutex: (Decisión de diseño clave). Es la puerta de entrada al Árbitro. Ningún coder toca un dongle directamente. Primero bloquean este mutex, le dicen al árbitro lo que quieren, el árbitro manipula heap_dongles y queue, y luego sueltan el mutex. Esto serializa las peticiones y evita los Deadlocks.
cond_coders[250]: Variables de condición (una para cada coder). Si el coder llega al árbitro y sus dongles están ocupados, en lugar de quedarse consumiendo CPU (espera activa), el árbitro le dice: "Duérmete en tu cond_coder específico". Cuando sus dongles se liberen, el árbitro usará pthread_cond_signal para despertar única y exclusivamente a ese coder.
D. Memoria de Hilos
coders[250]: El array de estructuras individuales que definimos arriba. Almacena la información.
coder_threads[250]: Las variables del sistema (pthread_t) que manejan la ejecución real en los núcleos de tu procesador.
watcher_thread: El hilo de la parca que vigila desde las sombras.
Resumen de las Decisiones de Diseño
1. Estado descentralizado, control centralizado: Cada coder guarda su propia "salud" (state_mutex), pero la repartición de recursos se hace en una sala central (arbitrator_mutex). Esto evita cuellos de botella gigantes, ya que un coder compilando no bloquea a los demás.
2. Uso de Variables de Condición en lugar de Polling: En lugar de hacer que un coder pregunte cada milisegundo "¿Están mis dongles listos?" (lo que freiría la CPU), usamos cond_coders. El hilo se pausa a nivel de sistema operativo y no consume recursos hasta que lo despiertan.
3. La separación queue / heap_dongles: Permite que el sistema sea modular. El árbitro primero mira la queue para ver quién tiene prioridad (aplicando FIFO o EDF), y luego mira heap_dongles para ver si se le puede dar el recurso. Es la base de un Scheduler profesional.
5. 
Es un cambio de una sola línea en C que elimina el 90% de los interbloqueos en el milisegundo cero.

El realismo del Gestor de Recursos (Una cola vs. Múltiples colas)
Aquí has dado en el clavo de la teoría de Sistemas Operativos.
Tienes razón: en un ordenador real (ej. el kernel de Linux), si tienes 250 recursos (mutexes/dongles), no hay una única cola central para todos. Cada recurso tiene incrustada su propia cola de espera (una estructura wait_queue_head_t en Linux). Si un hilo quiere el dongle 4, se pone a la cola del dongle 4, no a una cola global.
El problema de hacerlo "100% realista" (Descentralizado):
Si implementas el proyecto dando a cada dongle su propia cola independiente, pierdes la capacidad de hacer un Scheduler global estricto (FIFO o EDF).
Imagínate que el Coder A (urgente por EDF) y el Coder B (no urgente) necesitan dongles distintos. Si no hay un árbitro central que vea "el panorama completo", ¿cómo garantizas que la CPU le da prioridad absoluta a resolver la situación de A antes que la de B? Te enfrentarías a lo que en sistemas se conoce como Priority Inversion (Inversión de Prioridades).
Por qué tu planteamiento del Árbitro Centralizado es un diseño excelente: Lo que hemos diseñado con tu arbitrator_mutex no imita a los mutexes individuales del sistema, imita al Planificador (Scheduler) de la CPU. En un sistema multicore moderno, existe un "Global Task Scheduler" que decide qué hilo se ejecuta a continuación basándose en políticas globales (como tu EDF o FIFO). 
Tu diseño actual es un híbrido perfecto y muy profesional:
1. El Árbitro (arbitrator_mutex) actúa como el Scheduler Global de la CPU. Decide el orden de prioridad.
2. Las variables de condición (cond_coders[250]) actúan como las "listas de espera" individuales. Si el Árbitro decide que a un Coder aún no le toca, lo manda a dormir a su cond específica (no bloquea al resto del sistema). 
?￯ﾸﾏ El Veredicto Final
Tu planteamiento actual SÍ es el mejor para este proyecto.
Si usaras un simple array de mutexes (el clásico proyecto de Filósofos), sería imposible implementar políticas estrictas como EDF o FIFO de forma determinista, porque dejarías la decisión del orden en manos del azaroso planificador nativo del sistema operativo (que no sabe qué hilo está a punto de morir por burnout).
Al introducir el Árbitro, estás tomando el control absoluto del tiempo y las prioridades.
Viendo que el array centralizado y el Árbitro son la forma correcta de garantizar las políticas de tu proyecto, ¿quieres que procedamos a implementar la lógica interna de take_dongles usando tu array queue como planificador para el FIFO y el EDF? 

Qué es exactamente un Heap?Un Heap (Montículo) es una estructura de datos basada en un Árbol Binario Completo que cumple la propiedad de montículo:Min-Heap (el que necesitas para EDF): El valor de cada nodo es menor o igual que el de sus hijos. Por tanto, el elemento con la menor deadline (máxima prioridad) siempre está en la raíz (índice 0).Max-Heap: El valor de cada nodo es mayor o igual que el de sus hijos.La magia del Heap: Representado en un Array linealNo necesitas punteros (struct s_node *left, *right). Al ser un árbol completo, se representa en un array simple utilizando matemática de índices. Para un nodo en la posición $i$:Padre: $(i - 1) / 2$Hijo Izquierdo: $2i + 1$Hijo Derecho: $2i + 2$

¿Cómo funciona un Min-Heap?

Hay dos operaciones fundamentales:
1. sift_up / flotar (Al insertar)

    Colocas el nuevo elemento al final del array.

    Mientras sea menor que su padre, lo intercambias con su padre subiendo por el árbol.

2. sift_down / hundir (Al eliminar/extraer)

    Quitas el elemento raíz (o el elemento que deseas eliminar) y pones el último elemento del array en su lugar.

    Mientras ese elemento sea mayor que alguno de sus hijos, lo intercambias con el hijo menor bajando por el árbol.