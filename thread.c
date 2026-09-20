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
int find_unused_tcb() {
    for(int i = 1; i <= MAX_THREAD; i++) {
        if(TCB[(current_idx + i) % MAX_THREAD].status == THREAD_UNUSED) {
            return (current_idx + i) % MAX_THREAD;
        }
    }
    return -1;
}

int find_ready_tcb() {
    for(int i = 1; i <= MAX_THREAD; i++) {
        // Avoid directly finding itself
        if(TCB[(current_idx + i) % MAX_THREAD].status == THREAD_READY) {
            return (current_idx + i) % MAX_THREAD;
        }
    }
    return -1;
}
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

void thread_create(void (*entry)(void *arg), void *arg) {
    /* Student's code goes here (Cooperative Threads). */
    int child_idx = find_unused_tcb();
    int parent_idx = current_idx;
    if(child_idx < 0) {
        printf("No unusd TCB.\n");
        return;
    }

    char* child_stack = malloc(STACK_SIZE);
    if(!child_stack) {
        printf("No enought memory.\n");
        return;
    }
    TCB[child_idx].entry = entry;
    TCB[child_idx].arg = arg;
    TCB[child_idx].id = child_idx;  // Bind with real index
    TCB[child_idx].stack_base = child_stack;
    TCB[child_idx].status = THREAD_READY;

    TCB[parent_idx].status = THREAD_READY;  // yield CPU
    current_idx = child_idx;
    ctx_start(&TCB[parent_idx].sp, child_stack + STACK_SIZE);

    // Check if the child thread is zombie
    if(TCB[child_idx].status == THREAD_ZOMBIE) {
        free(TCB[child_idx].stack_base);
        TCB[child_idx].stack_base = NULL;
        TCB[child_idx].sp = NULL;
        TCB[child_idx].status = THREAD_UNUSED;
    }
    current_idx = parent_idx;
    TCB[parent_idx].status = THREAD_RUNNING;
    /* Student's code ends here. */
}

void thread_yield() {
    /* Student's code goes here (Cooperative Threads). */
    int self_idx = current_idx;
    int next_idx = find_ready_tcb();
    if(next_idx == -1) {
        if(TCB[self_idx].status == THREAD_ZOMBIE) {
            TCB[self_idx].status = THREAD_UNUSED;
            _end();
        }
        else if(TCB[self_idx].status == THREAD_WAITING) {
            _end();
        }
        return;
    }

    if(TCB[self_idx].status == THREAD_RUNNING) {
        TCB[self_idx].status = THREAD_READY;  // yield CPU
    }
    TCB[next_idx].status = THREAD_RUNNING;
    current_idx = next_idx;
    ctx_switch(&TCB[self_idx].sp, TCB[current_idx].sp);
    
    // Check if the yielding-thread is zombie
    if(TCB[next_idx].status == THREAD_ZOMBIE) {
        free(TCB[next_idx].stack_base);
        TCB[next_idx].stack_base = NULL;
        TCB[next_idx].sp = NULL;
        TCB[next_idx].status = THREAD_UNUSED;
    }
    current_idx = self_idx;
    TCB[self_idx].status = THREAD_RUNNING;
    /* Student's code ends here. */
}

void thread_exit() {
    /* Student's code goes here (Cooperative Threads). */
    TCB[current_idx].arg = NULL;
    TCB[current_idx].entry = NULL;
    TCB[current_idx].id = -1;
    TCB[current_idx].status = THREAD_ZOMBIE;
    thread_yield();
    /* Student's code ends here. */
}

/* Student's code goes here (Cooperative Threads). */
/* Define helper functions (if needed) for conditional variables. */

/* Student's code ends here. */

void cv_init(struct cv *condition) {
    /* Student's code goes here (Cooperative Threads). */
    for(int i = 0; i < MAX_THREAD; i++) {
        condition->waiter[i] = -1;
    }
    condition->count = 0;
    /* Student's code ends here. */
}

void cv_wait(struct cv *condition) {
    /* Student's code goes here (Cooperative Threads). */
    condition->waiter[condition->count++] = current_idx;
    TCB[current_idx].status = THREAD_WAITING;
    thread_yield();
    TCB[current_idx].status = THREAD_RUNNING;
    /* Student's code ends here. */
}

void cv_signal(struct cv *condition) {
    /* Student's code goes here (Cooperative Threads). */
    if(condition->count == 0) {
        return;
    }
    int waitting_idx = condition->waiter[--condition->count];
    TCB[waitting_idx].status = THREAD_READY;
    condition->waiter[condition->count] = -1;
    /* Student's code ends here. */
}

#define BUF_SIZE 3
void* buffer[BUF_SIZE];
int count = 0;
int head = 0, tail = 0;
struct cv nonempty, nonfull;

void produce(void* arg) {
    int ID = *((int*)(arg));
    while (1) {
        while (count == BUF_SIZE) cv_wait(&nonfull);
        /* At this point, the buffer is not full. */

        /* Student's code goes here (Cooperative Threads). */
        printf("Producer-Thread ID: %d.\n", ID);
        /* Print out the producer ID using the arg parameter. */

        /* Student's code ends here. */
        buffer[tail] = arg;
        tail = (tail + 1) % BUF_SIZE;
        count += 1;
        cv_signal(&nonempty);
    }
}

void consume(void *arg) {
    int ID = *((int*)(arg));
    while (1) {
        while (count == 0) cv_wait(&nonempty);
        /* At this point, the buffer is not empty. */

        /* Student's code goes here (Cooperative Threads). */
        printf("Consumer-Thread ID: %d.\n", ID);
        /* Print out the consumer ID using the arg parameter. */

        /* Student's code ends here. */
        void* result = buffer[head];
        head = (head + 1) % BUF_SIZE;
        count -= 1;
        cv_signal(&nonfull);
    }
}

// Condition Variable
int main() {
    thread_init();
    cv_init(&nonfull);
    cv_init(&nonempty);

    #define MAX_ID 2
    int ID[MAX_ID];
    for (int i = 0; i < MAX_ID; i++) ID[i] = i;

    for (int i = 0; i < MAX_ID; i++)
        thread_create(consume, ID + i);

    for (int i = 0; i < MAX_ID; i++)
        thread_create(produce, ID + i);

    printf("main thread exits\n\r");
    thread_exit();

    /* The control flow should NEVER get here. If the main thread is the last to
     * call thread_exit(), thread_exit() should terminate the program by calling
     * the _end() in thread.s.
     * If the main thread is not the last, thread_exit() will switch the context
     * to another thread. Later, when all threads have called thread_exit(), the
     * last exited thread should then call _end() when running thread_exit(). */
}

// Linear Execute
/* void child(void* arg) {
    printf("%s is running.\n\r", arg);
}

int main() {
    thread_init();
    thread_create(child, "Child thread");
    printf("Main thread is running.\n\r");
    thread_exit();
} */


// Interleaving Execute
/* void child(void* arg) {
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
} */