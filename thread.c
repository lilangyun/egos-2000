/*
 * (C) 2026, Cornell University
 * All rights reserved.
 *
 * Description: cooperative threads and synchronization
 */

#include <sys/queue.h>
#include "print.c"
#include "thread.h"

/* Student's code goes here (Cooperative Threads). */
/* Define the TCB and helper functions (if needed) for cooperative threads. */

/* Student's code ends here. */

void thread_init() {
    /* Student's code goes here (Cooperative Threads). */
    for(int i = 0; i < MAX_THREAD; i++) {
        TCB[i].arg = NULL;
        TCB[i].entry = NULL;
        TCB[i].id = -1;
        TCB[i].sp = NULL;
        TCB[i].stack_base = NULL;
        TCB[i].status = THREAD_UNUSED;
    }
    current_idx = 0;
    TCB[current_idx].id = 0;
    TCB[current_idx].status = THREAD_RUNNING;
    /* Student's code ends here. */
}

void ctx_entry() {
    /* Student's code goes here (Cooperative Threads). */
    TCB[current_idx].status = THREAD_RUNNING;
    TCB[current_idx].entry(TCB[current_idx].arg);
    thread_exit();
    /* Student's code ends here. */
}

int find_unused_tcb() {
    for(int i = 0; i < MAX_THREAD; i++) {
        if(TCB[i].status == THREAD_UNUSED) {
            return i;
        }
    }
    return -1;
}

void reclaim_zombies() {
    for(int i = 0; i < MAX_THREAD; i++) {
        if(TCB[i].status == THREAD_ZOMBIE) {
            free(TCB[i].stack_base);
            TCB[i].stack_base = NULL;
            TCB[i].status = THREAD_UNUSED;
        }
    }
}

void thread_create(void (*entry)(void *arg), void *arg) {
    /* Student's code goes here (Cooperative Threads). */
    int child_idx = find_unused_tcb();
    int parent_idx = current_idx;
    if(child_idx < 0) {
        printf("No unusd TCB.\n");
    }
    TCB[child_idx].entry = entry;
    TCB[child_idx].arg = arg;
    TCB[child_idx].id = child_idx;  // Bind with real index
    reclaim_zombies();
    char* child_stack = malloc(STACK_SIZE);
    TCB[child_idx].stack_base = child_stack;
    TCB[child_idx].status = THREAD_READY;
    TCB[parent_idx].status = THREAD_READY;  // yield CPU
    current_idx = child_idx;
    ctx_start(&TCB[parent_idx].sp, child_stack + STACK_SIZE);
    TCB[parent_idx].status = THREAD_RUNNING;
    /* Student's code ends here. */
}

int find_ready_tcb() {
    for(int i = 0; i < MAX_THREAD; i++) {
        if(TCB[i].status == THREAD_READY) {
            return i;
        }
    }
    return -1;
}

void thread_yield() {
    /* Student's code goes here (Cooperative Threads). */
    int next_idx = find_ready_tcb();
    int self_idx = current_idx;
    TCB[self_idx].status = THREAD_READY;  // yield CPU
    TCB[next_idx].status = THREAD_RUNNING;
    current_idx = next_idx;
    ctx_switch(&TCB[self_idx].sp, TCB[current_idx].sp);
    /* Student's code ends here. */
}

void thread_exit() {
    /* Student's code goes here (Cooperative Threads). */
    TCB[current_idx].arg = NULL;
    TCB[current_idx].entry = NULL;
    TCB[current_idx].id = -1;
    TCB[current_idx].sp = NULL;
    TCB[current_idx].status = THREAD_ZOMBIE;
    int zombie_idx = current_idx;
    current_idx = find_ready_tcb();
    if(current_idx == -1) {
        TCB[zombie_idx].status = THREAD_UNUSED;
        _end();
    }
    TCB[current_idx].status = THREAD_RUNNING;
    ctx_switch(&TCB[zombie_idx].sp, TCB[current_idx].sp);
    /* Student's code ends here. */
}

/* Student's code goes here (Cooperative Threads). */
/* Define helper functions (if needed) for conditional variables. */

/* Student's code ends here. */

void cv_init(struct cv *condition) {
    /* Student's code goes here (Cooperative Threads). */

    /* Student's code ends here. */
}

void cv_wait(struct cv *condition) {
    /* Student's code goes here (Cooperative Threads). */

    /* Student's code ends here. */
}

void cv_signal(struct cv *condition) {
    /* Student's code goes here (Cooperative Threads). */

    /* Student's code ends here. */
}

#define BUF_SIZE 3
void* buffer[BUF_SIZE];
int count = 0;
int head = 0, tail = 0;
struct cv nonempty, nonfull;

void produce(void* arg) {
    while (1) {
        while (count == BUF_SIZE) cv_wait(&nonfull);
        /* At this point, the buffer is not full. */

        /* Student's code goes here (Cooperative Threads). */
        /* Print out the producer ID using the arg parameter. */

        /* Student's code ends here. */
        buffer[tail] = arg;
        tail = (tail + 1) % BUF_SIZE;
        count += 1;
        cv_signal(&nonempty);
    }
}

void consume(void *arg) {
    while (1) {
        while (count == 0) cv_wait(&nonempty);
        /* At this point, the buffer is not empty. */

        /* Student's code goes here (Cooperative Threads). */
        /* Print out the consumer ID using the arg parameter. */

        /* Student's code ends here. */
        void* result = buffer[head];
        head = (head + 1) % BUF_SIZE;
        count -= 1;
        cv_signal(&nonfull);
    }
}

// int main() {
//     thread_init();
//     cv_init(&nonfull);
//     cv_init(&nonempty);

//     int ID[500];
//     for (int i = 0; i < 500; i++) ID[i] = i;

//     for (int i = 0; i < 500; i++)
//         thread_create(consume, ID + i);

//     for (int i = 0; i < 500; i++)
//         thread_create(produce, ID + i);

//     printf("main thread exits\n\r");
//     thread_exit();

//     /* The control flow should NEVER get here. If the main thread is the last to
//      * call thread_exit(), thread_exit() should terminate the program by calling
//      * the _end() in thread.s.
//      * If the main thread is not the last, thread_exit() will switch the context
//      * to another thread. Later, when all threads have called thread_exit(), the
//      * last exited thread should then call _end() when running thread_exit(). */
// }

// void child(void* arg) {
//     printf("%s is running.\n\r", arg);
// }

// int main() {
//     thread_init();
//     thread_create(child, "Child thread");
//     printf("Main thread is running.\n\r");
//     thread_exit();
// }

void child(void* arg) {
    for (int i = 0; i < 10; i++) {
        printf("%s is in for loop i=%d\n\r", arg, i);
        thread_yield();
    }
}

int main() {
    thread_init();
    thread_create(child, "Child thread");
    for (int i = 0; i < 10; i++) {
        printf("Main thread is in for loop i=%d\n\r", i);
        thread_yield();
    }
    thread_exit();
}