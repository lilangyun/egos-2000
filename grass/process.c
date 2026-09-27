/*
 * (C) 2026, Cornell University
 * All rights reserved.
 *
 * Description: helper functions for process management
 */

#include "process.h"

#define MLFQ_NLEVELS          5
#define MLFQ_RESET_PERIOD     100000000         /* 10 seconds */
#define MLFQ_LEVEL_RUNTIME(x) (x + 1) * 1000000 /* e.g., 100ms for level 0 */
extern struct process proc_set[MAX_NPROCESS + 1];

static void proc_set_status(int pid, enum proc_status status) {
    for (uint i = 0; i < MAX_NPROCESS; i++)
        if (proc_set[i].pid == pid) proc_set[i].status = status;
}

static void proc_print_lifecycle(int idx) {
    if(idx >= MAX_NPROCESS) return;
    proc_set[idx].clock_termination = mtime_get();
    proc_set[idx].time_turnaround = proc_set[idx].clock_termination - proc_set[idx].clock_creation;
    INFO("process %u terminated after %d timer interrupts, turnaround time: %llums, response time: %llums, CPU time: %llums", 
        proc_set[idx].pid, proc_set[idx].num_timer_interrupt, 
        proc_set[idx].time_turnaround / 10000,
        proc_set[idx].time_response / 10000,
        proc_set[idx].time_running / 10000 );
}

static uint pid_to_idx(int pid) {
    for (uint i = 0; i < MAX_NPROCESS; i++)
        if (proc_set[i].pid == pid) return i;
    return MAX_NPROCESS;
}

void proc_set_ready(int pid) { proc_set_status(pid, PROC_READY); }
void proc_set_running(int pid) { proc_set_status(pid, PROC_RUNNING); }
void proc_set_runnable(int pid) { proc_set_status(pid, PROC_RUNNABLE); }
void proc_set_pending(int pid) { proc_set_status(pid, PROC_PENDING_SYSCALL); }

int proc_alloc() {
    static uint curr_pid = 0;
    for (uint i = 0; i < MAX_NPROCESS; i++)
        if (proc_set[i].status == PROC_UNUSED) {
            proc_set[i].pid    = ++curr_pid;
            proc_set[i].status = PROC_LOADING;
            /* Student's code goes here (Multiple Projects). */

            /* [Preemptive Scheduling]
             * Initialize the fields for lifecycle statistics and MLFQ.
             * [System Call & Protection]
             * Initialize the fields for the process sleep system call. */
            ulonglong clock_now = mtime_get();
            proc_set[i].clock_creation = clock_now;
            proc_set[i].clock_response = 0;
            proc_set[i].clock_switch_in = 0;
            proc_set[i].time_running = 0;
            proc_set[i].num_timer_interrupt = 0;

            proc_set[i].mlfq_level = 0;
            proc_set[i].mlfq_time_running = 0;

            proc_set[i].time_sleep = 0;
            proc_set[i].clock_sleep = 0;
            /* Student's code ends here. */
            return curr_pid;
        }

    FATAL("proc_alloc: reach the limit of %d processes", MAX_NPROCESS);
}

void proc_free(int pid) {
    /* Student's code goes here (Preemptive Scheduling). */

    /* Print the lifecycle statistics of the terminated process or processes. */
    if (pid != GPID_ALL) {
        uint idx = pid_to_idx(pid);
        proc_print_lifecycle(idx);
        earth->mmu_free(pid);
        proc_set_status(pid, PROC_UNUSED);
    } else {
        /* Free all user processes. */
        for (uint i = 0; i < MAX_NPROCESS; i++)
            if (proc_set[i].pid >= GPID_USER_START &&
                proc_set[i].status != PROC_UNUSED) {
                proc_print_lifecycle(i);
                earth->mmu_free(proc_set[i].pid);
                proc_set[i].status = PROC_UNUSED;
            }
    }
    /* Student's code ends here. */
}

void mlfq_update_level(struct process* p, ulonglong runtime) {
    /* Student's code goes here (Preemptive Scheduling). */

    /* Update the MLFQ-related fields in struct process* p after this
     * process has run on the CPU for another ulonglong runtime time units. */
    uint mlfq_level = p->mlfq_level;
    if(mlfq_level < (MLFQ_NLEVELS-1) && runtime >= MLFQ_LEVEL_RUNTIME(mlfq_level)) {
        p->mlfq_level++;
        p->mlfq_time_running = 0;
    }

    /* Student's code ends here. */
}

void mlfq_reset_level() {
    /* Student's code goes here (Preemptive Scheduling). */
    if (!earth->tty_input_empty()) {
        /* Reset the level of GPID_SHELL if there is pending keyboard input. */
        uint idx = pid_to_idx(GPID_SHELL);
        if(idx < MAX_NPROCESS) {
            proc_set[idx].mlfq_level = 0;
            proc_set[idx].mlfq_time_running = 0;
        }
    }

    static ulonglong MLFQ_last_reset_time = 0;
    /* Reset the level of all processes every MLFQ_RESET_PERIOD time units. */
    ulonglong clock_now = mtime_get();
    if(clock_now - MLFQ_last_reset_time >= MLFQ_RESET_PERIOD) {
        for(uint i = 0; i < MAX_NPROCESS; i++) {
            proc_set[i].mlfq_level = 0;
            proc_set[i].mlfq_time_running = 0;
        }
        MLFQ_last_reset_time = clock_now;
    }
    /* Student's code ends here. */
}

void proc_sleep(int pid, uint usec) {
    /* Student's code goes here (System Call & Protection). */

    /* Update the sleep-related fields in the struct process for process pid. */
    uint idx = pid_to_idx(pid);
    if (idx >= MAX_NPROCESS) return;

    proc_set[idx].clock_sleep = mtime_get();
    proc_set[idx].time_sleep = usec * (ulonglong)10;
    /* Student's code ends here. */
}

void proc_coresinfo() {
    /* Student's code goes here (Multicore & Locks). */

    /* Print out the pid of the process running on each CPU core. */

    /* Student's code ends here. */
}
