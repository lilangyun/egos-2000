/*
 * (C) 2026, Cornell University
 * All rights reserved.
 *
 * Description: cooperative threads and synchronization
 */

#include "print.c"
#include "thread.h"

/* Student's code goes here (Cooperative Threads). */
/* Define the TCB and helper functions (if needed) for cooperative threads. */
int alloc_tid() {
    for(int i = 0; i < MAX_THREAD; i++) {
        if(!tid_status[i]) {
            tid_status[i] = 1;
            return i;
        }
    }
    return -1;    
}

int release_tid(int tid) {
    tid_status[tid] = 0;
}

struct thread* find_ready_thread() {
    struct thread* iter = TAILQ_NEXT(current_thread, ptr);
    while (iter != NULL) {
        if (iter->status == THREAD_READY) return iter;
        iter = TAILQ_NEXT(iter, ptr);
    }
    TAILQ_FOREACH(iter, &TCB, ptr) {
        if (iter->status == THREAD_READY) return iter;
    }
    return NULL;
}
/* Student's code ends here. */

void thread_init() {
    /* Student's code goes here (Cooperative Threads). */
    TAILQ_INIT(&TCB);
    struct thread *t = malloc(sizeof(struct thread));
    t->id = alloc_tid();
    t->status = THREAD_RUNNING;
    t->entry = NULL;
    t->arg = NULL;
    t->stack_base = NULL;
    t->sp = NULL;
    TAILQ_INSERT_TAIL(&TCB, t, ptr);
    current_thread = t;
    /* Student's code ends here. */
}

void ctx_entry() {
    /* Student's code goes here (Cooperative Threads). */
    current_thread->status = THREAD_RUNNING;
    current_thread->entry(current_thread->arg);
    thread_exit();
    /* Student's code ends here. */
}

void thread_create(void (*entry)(void *arg), void *arg) {
    /* Student's code goes here (Cooperative Threads). */
    struct thread *parent_thread = current_thread;
    struct thread *child_thread = malloc(sizeof(struct thread));
    char* child_stack = malloc(STACK_SIZE);
    if((!child_thread) || (!child_stack)) {
        printf("No enought memory.\n");
        return;
    }
    child_thread->entry = entry;
    child_thread->arg = arg;
    if((child_thread->id = alloc_tid()) == -1) {
        printf("No free tid.\n");
        return;
    }

    child_thread->stack_base = child_stack;
    child_thread->status = THREAD_READY;
    TAILQ_INSERT_TAIL(&TCB, child_thread, ptr);

    parent_thread->status = THREAD_READY;  // yield CPU
    current_thread = child_thread;
    ctx_start(&parent_thread->sp, child_stack + STACK_SIZE);

    // Check if the child thread is zombie
    if(child_thread->status == THREAD_ZOMBIE) {
        TAILQ_REMOVE(&TCB, child_thread, ptr);
        release_tid(child_thread->id);
        free(child_thread->stack_base);
        free(child_thread);
    }
    /* Student's code ends here. */
}

void thread_yield() {
    /* Student's code goes here (Cooperative Threads). */
    struct thread *self_thread = current_thread;
    struct thread *next_thread = find_ready_thread();
    if(next_thread == NULL) {
        if(self_thread->status == THREAD_ZOMBIE) {
            self_thread->status = THREAD_UNUSED;
            _end();
        }
        return;
    }

    if(self_thread->status== THREAD_RUNNING) {
        self_thread->status = THREAD_READY;  // yield CPU
    }
    next_thread->status = THREAD_RUNNING;
    current_thread = next_thread;
    ctx_switch(&self_thread->sp, current_thread->sp);
    
    // Check if the yielding-thread is zombie
    if(next_thread->status == THREAD_ZOMBIE) {
        TAILQ_REMOVE(&TCB, next_thread, ptr);
        release_tid(next_thread->id);
        free(next_thread->stack_base);
        free(next_thread);
    }
    /* Student's code ends here. */
}

void thread_exit() {
    /* Student's code goes here (Cooperative Threads). */
    current_thread->status = THREAD_ZOMBIE;
    thread_yield();
    /* Student's code ends here. */
}

/* Student's code goes here (Cooperative Threads). */
/* Define helper functions (if needed) for conditional variables. */

/* Student's code ends here. */

void cv_init(struct cv *condition) {
    /* Student's code goes here (Cooperative Threads). */
    TAILQ_INIT(&condition->waiter);
    /* Student's code ends here. */
}

void cv_wait(struct cv *condition) {
    /* Student's code goes here (Cooperative Threads). */
    TAILQ_REMOVE(&TCB, current_thread, ptr);
    TAILQ_INSERT_TAIL(&(condition->waiter), current_thread, cv_ptr);
    current_thread->status = THREAD_WAITING;
    thread_yield();
    current_thread->status = THREAD_RUNNING;
    /* Student's code ends here. */
}

void cv_signal(struct cv *condition) {
    /* Student's code goes here (Cooperative Threads). */
    struct thread* first_wait = TAILQ_FIRST(&condition->waiter);
    if(first_wait) {
        TAILQ_REMOVE(&condition->waiter, first_wait, cv_ptr);
        TAILQ_INSERT_TAIL(&TCB, first_wait, ptr);
        first_wait->status = THREAD_READY;
        thread_yield();
    }
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

// // Condition Variable
int main() {
    thread_init();
    cv_init(&nonfull);
    cv_init(&nonempty);

    #define MAX_ID 100
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