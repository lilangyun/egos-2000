#pragma once

#include "egos.h"
#include "syscall.h"

enum proc_status {
    PROC_UNUSED,
    PROC_LOADING,
    PROC_READY,
    PROC_RUNNING,
    PROC_RUNNABLE,
    PROC_PENDING_SYSCALL,
    /* A process the kernel has killed (e.g., after a fault). It must never be
     * scheduled again; GPID_PROCESS reaps it with proc_free(). */
    PROC_ZOMBIE,
};

struct process {
    int pid;
    struct syscall syscall;
    enum proc_status status;
    uint mepc, saved_registers[32];
    /* Student's code goes here (Preemptive Scheduling | System Call). */
    
    /* Add new fields for lifecycle statistics, MLFQ, or process sleep. */
    ulonglong clock_creation, clock_response, clock_termination;
    ulonglong clock_switch_in;
    ulonglong time_turnaround, time_response, time_running;
    uint num_timer_interrupt;

    uint mlfq_level;
    ulonglong mlfq_time_running;

    ulonglong time_sleep;
    ulonglong clock_sleep;
    /* Student's code ends here. */
};
#define MAX_NPROCESS 16

ulonglong mtime_get();

int proc_alloc();
void proc_free(int);
void proc_set_ready(int);
void proc_set_running(int);
void proc_set_runnable(int);
void proc_set_pending(int);
void proc_set_zombie(int);

void mlfq_reset_level();
void mlfq_update_level(struct process* p, ulonglong runtime);
void proc_sleep(int pid, uint usec);
void proc_coresinfo();

extern uint core_to_proc_idx[NCORES];
