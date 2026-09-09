*This project has been created as part of the 42 curriculum by adaza-ru.*

# Codexion

## Description

Codexion is a concurrency simulation written in C using POSIX threads.


This project is an evolution of the **Dining Philosophers problem**, originally introduced by **Edsger Dijkstra**. Instead of philosophers competing for forks, coders compete for USB dongles while trying to compile before reaching their burnout deadline.

Several coders share a circular set of USB dongles. Each coder must acquire two adjacent dongles before compiling. After compiling, the coder releases the dongles, debugs, refactors, and then tries to compile again.

The simulation ends when every coder has completed the required number of compilations or when a coder burns out because they did not start compiling before their deadline.

The project focuses on:

- POSIX threads
- Mutexes and condition variables
- Resource synchronization
- FIFO and EDF scheduling
- Priority queues implemented with a binary heap
- Dongle cooldown periods
- Precise burnout detection
- Thread-safe logging

### Features

- One thread per coder
- One monitor thread for burnout detection
- Circular dongle topology
- FIFO scheduling
- Earliest Deadline First scheduling
- Binary heap for waiting coders
- Dongle cooldown support
- Serialized output
- Graceful simulation termination
- Validation of all command-line arguments

## Instructions

The project requires a C compiler, POSIX threads, and Make.

### Makefile commands:

```bash
	all / color			build (plain / with colored output)
	clean				to remove object files
	fclean				to remove object files and the executable
	re/recolor			to rebuild the project
	termtest			quick, tool-free demo: some errors and base behaviour
	logtest				full matrix (memcheck/helgrind/tsan/asan), written to "logs/"
	logtest-errors		just the argument-validation matrix, under memcheck
	logtest-memcheck	just the memcheck matrix
	logtest-helgrind	just the helgrind matrix
	logtest-tsan		just the ThreadSanitizer matrix
	logtest-asan		just the AddressSanitizer matrix
```

### Execution
The program requires exactly eight arguments:
``./codexion <number_of_coders> <time_to_burnout> <time_to_compile> <time_to_debug> <time_to_refactor> <number_of_compiles_required> <dongle_cooldown> <scheduler>``

Arguments:

- ``number_of_coders``: Number of coders and dongles. Must be between 1 and 250.
- ``time_to_burnout``: Maximum time in milliseconds before a coder burns out.
- ``time_to_compile``: Compilation duration in milliseconds.
- ``time_to_debug``: Debugging duration in milliseconds.
- ``time_to_refactor``: Refactoring duration in milliseconds.
- ``number_of_compiles_required``: Number of compilations required per coder to end the simulation successfully.
- ``dongle_cooldown``: Time in milliseconds during which a released dongle remains unavailable.
- ``scheduler``: Either fifo or edf.

Example: ``./codexion 6 800 200 100 100 3 100 edf``

### Scheduling policies
``fifo`` uses **First In, First Out** scheduling. Waiting coders are served according to the order in which they entered the queue.

``edf`` uses **Earliest Deadline First** scheduling. A coder's deadline is calculated as: last_compile_start + time_to_burnout


## Blocking cases handled

### Deadlock prevention
Coders acquire both required dongles atomically while holding the arbitrator mutex. A coder never holds one dongle while waiting for another.

This prevents the Coffman conditions associated with deadlock:

- **Mutual exclusion**: Dongles are exclusive resources, but access is centrally coordinated.
- **Hold and wait**: Coders do not acquire one dongle and then wait for another.
- **No preemption**: Dongles are not forcibly taken from a coder during compilation.
- **Circular wait**: Coders do not acquire dongles independently, so circular waiting cannot form.

The two dongles are marked as taken at the same time while the arbitrator mutex is locked.

### Starvation prevention
Waiting coders are stored in a priority heap. 
* FIFO uses arrival order
* EDF uses the earliest burnout deadline.

Condition variables wake waiting coders when dongles are released. The priority check is performed again after every wake-up, so a coder cannot permanently bypass the scheduling policy.

### Dongle cooldown
After a coder releases its dongles, each dongle receives a future availability timestamp:

``dongle_free_at = current_time + dongle_cooldown``

A dongle cannot be acquired before this timestamp.

### Burnout detection
A separate monitor thread checks every coder's last compilation start time.

A coder burns out when:

``current_time - last_compile_start >= time_to_burnout``

The monitor sets the simulation termination flag and wakes all waiting coder threads when burnout occurs.

### Simulation termination
The simulation stops when either:

- A coder burns out.
- Every coder reaches number_of_compiles_required.

All waiting coders are notified when the simulation ends, allowing the threads to terminate instead of remaining blocked.

### Log serialization
All output is protected by ``write_mutex``. This ensures that two messages cannot be written simultaneously and prevents interleaved output lines.

Each state change is printed in the following format: ``timestamp coder_id action``

The monitor also uses the same mutex when printing burnout or completion messages.

## Thread synchronization mechanisms

### ``pthread_mutex_t``
The program uses several mutexes with separate responsibilities:

* ``arbitrator_mutex``: Protects dongle state, the waiting heap, arrival sequence numbers, and condition-variable coordination.

* ``write_mutex``: Serializes all output.

* ``end_mutex``: Protects the simulation_end flag.

* ``state_mutex``: Protects each coder's last_compile_start and compiles_done fields.

### ``pthread_cond_t``
Each coder has an associated condition variable.

A coder waits on its condition variable when:

- One of its dongles is currently taken.
- One of its dongles is still cooling down.
- Another waiting coder has higher priority.

The condition wait temporarily releases arbitrator_mutex, allowing another thread to release dongles and signal waiting coders.

When dongles are released, neighboring coders are notified with:

``pthread_cond_broadcast(&env->cond_coders[neighbor_id]);``

The waiting coder then reacquires ``arbitrator_mutex`` and checks the conditions again.

### Communication between coders and the monitor
The monitor and coder threads communicate through shared state protected by mutexes:

- *Coders* update ``last_compile_start`` when compilation begins.
- *Coders* increment ``compiles_done`` after compilation finishes.
- The *monitor* reads both values while holding state_mutex.
- The *monitor* writes ``simulation_end`` while holding end_mutex.
- Waiting coders periodically check ``simulation_end`` and are explicitly woken through condition variables when the simulation stops.

This implementation does not define a separate custom event object. Instead, it implements event-style communication using shared state, mutexes, and condition variables. The ``simulation_end`` flag acts as the event state, while condition variables wake waiting coder threads.

### Data structures
The waiting queue is implemented as a **Binary Min-Heap** stored in a fixed-size array.

The heap priority depends on the selected scheduler.


## Resources

* [CodeVault - Unix Threads in C](https://www.youtube.com/playlist?list=PLfqABt5AS4FmuQf70psXrsMLEDQXNkLq2)

* **Modern Operating Systems 4th Edition -- Andrew Tanenbaum** Chapter 2: Processes And Threads

* **Operating Systems: Three Easy Pieces -- Remzi H. Arpaci-Dusseau && Andrea C. Arpaci-Dusseau**

* [POSIX Threads documentation](https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/pthread.h.html)
* [Linux pthread_create documentation](https://man7.org/linux/man-pages/man3/pthread_create.3.html)
* [Linux pthread condition variables](https://man7.org/linux/man-pages/man3/pthread_cond_wait.3.html)
* [Linux pthread mutex documentation](https://man7.org/linux/man-pages/man3/pthread_mutex_lock.3p.html)
* [gettimeofday documentation](https://man7.org/linux/man-pages/man2/gettimeofday.2.html)

### AI usage
AI tools were used as a review and learning aid during the project.

They helped with:

- Writing this README.
- Reviewing thread synchronization logic.
- Checking FIFO and EDF scheduling behaviour.
- Understanding timing issues involving burnout deadlines and dongle cooldowns.
- Suggesting test cases for argument validation and high thread counts.
- Interpreting Valgrind, Helgrind, and thread-creation diagnostics.


All generated suggestions were reviewed, tested, and adapted to the project's implementation.


