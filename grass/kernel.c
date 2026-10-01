/*
 * (C) 2026, Cornell University
 * All rights reserved.
 *
 * Description: kernel ≈ 2 handlers
 *   intr_entry() handles timer and device interrupts.
 *   excp_entry() handles system calls and faults (e.g., invalid memory access).
 */

#include "process.h"
#include <string.h>

uint core_in_kernel;
uint core_to_proc_idx[NCORES];
struct process proc_set[MAX_NPROCESS + 1];
/* proc_set[MAX_NPROCESS] is a placeholder for idle cores. */

#define curr_proc_idx core_to_proc_idx[core_in_kernel]
#define curr_pid      proc_set[curr_proc_idx].pid
#define curr_status   proc_set[curr_proc_idx].status
#define curr_saved    proc_set[curr_proc_idx].saved_registers

static void intr_entry(uint);
static void excp_entry(uint);

void kernel_entry() {
    /* With the kernel lock, only one core can enter this point at any time. */
    asm("csrr %0, mhartid" : "=r"(core_in_kernel));

    /* Save the process context. */
    asm("csrr %0, mepc" : "=r"(proc_set[curr_proc_idx].mepc));
    memcpy(curr_saved, (void*)(EGOS_STACK_TOP - 32 * 4), 32 * 4);

    uint mcause;
    asm("csrr %0, mcause" : "=r"(mcause));
    (mcause & (1 << 31)) ? intr_entry(mcause & 0x3FF) : excp_entry(mcause);

    /* Restore the process context. */
    asm("csrw mepc, %0" ::"r"(proc_set[curr_proc_idx].mepc));
    memcpy((void*)(EGOS_STACK_TOP - 32 * 4), curr_saved, 32 * 4);
}

#define INTR_ID_TIMER   7
#define EXCP_ID_ECALL_U 8
#define EXCP_ID_ECALL_M 11
static void proc_yield();
static void proc_kill(uint id);
static void proc_try_syscall(struct process* proc);

static void excp_entry(uint id) {
    if (id >= EXCP_ID_ECALL_U && id <= EXCP_ID_ECALL_M) {
        /* Copy the system call arguments from user space to the kernel. */
        uint syscall_paddr = earth->mmu_translate(curr_pid, SYSCALL_ARG);
        memcpy(&proc_set[curr_proc_idx].syscall, (void*)syscall_paddr,
               sizeof(struct syscall));
        proc_set[curr_proc_idx].syscall.status = PENDING;

        proc_set_pending(curr_pid);
        proc_set[curr_proc_idx].mepc += 4;
        proc_try_syscall(&proc_set[curr_proc_idx]);
        proc_yield();
        return;
    }
    /* Student's code goes here (System Call & Protection | Virtual Memory). */

    /* Kill the current process if curr_pid is a user application. */
    if (curr_proc_idx < MAX_NPROCESS && curr_pid >= GPID_USER_START) {
        proc_kill(id);
        proc_yield();
        return;
    }
    /* Student's code ends here. */
    FATAL("excp_entry: kernel got exception %d", id);
}

/* Terminate the current user process. The process is not freed here but marked
 * as a zombie, and a PROC_EXIT message is sent to GPID_PROCESS on its behalf.
 * GPID_PROCESS reaps the process with proc_free() and, if a shell process is
 * waiting for it, sends a reply to the shell, which otherwise would wait
 * forever for a reply that never comes. */
static void proc_kill(uint id) {
    struct process* proc = &proc_set[curr_proc_idx];

    INFO("process %d terminated with exception %u", proc->pid, id);

    /* A zombie is never scheduled again; proc_yield() keeps retrying the
     * message until GPID_PROCESS is ready to receive it. */
    proc_set_zombie(proc->pid);
    memset(&proc->syscall, 0, sizeof(struct syscall));
    proc->syscall.type     = SYS_SEND;
    proc->syscall.receiver = GPID_PROCESS;
    proc->syscall.status   = PENDING;
    ((struct proc_request*)proc->syscall.content)->type = PROC_EXIT;
    proc_try_syscall(proc);
}

static void intr_entry(uint id) {
    /* Student's code goes here (Preemptive Scheduling). */
    
    /* Update the process lifecycle statistics. */
    if (id == INTR_ID_TIMER) proc_set[curr_proc_idx].num_timer_interrupt++;
    /* Student's code ends here. */

    if (id == INTR_ID_TIMER) return proc_yield();

    /* Student's code goes here (Ethernet & TCP/IP). */

    /* Handle an external interrupt from the Intel Gigabit Ethernet Controller.
     * Specifically, you need to (1) Claim the PLIC interrupt; (2) Check if the
     * interrupt is for receiving an Ethernet frame; (3) Read the received frame
     * from an RX buffer and print the content; (4) Complete the PLIC interrupt,
     * so PLIC can fire the next interrupt; (5) Call proc_yield() and return. */

    /* Student's code ends here. */
}

static void proc_yield() {
    if (curr_status == PROC_RUNNING) proc_set_runnable(curr_pid);

    /* Student's code goes here (Multiple Projects). */

    /* [Preemptive Scheduling]
     * Measure and record lifecycle statistics for the *current* process.
     * Modify the loop below to find the next process to schedule with MLFQ.
     * [System Call & Protection]
     * Do not schedule a process that should still be sleeping at this time. */
    ulonglong clock_now = mtime_get();
    ulonglong this_round_time = clock_now - proc_set[curr_proc_idx].clock_switch_in;
    proc_set[curr_proc_idx].time_running += this_round_time;
    proc_set[curr_proc_idx].mlfq_time_running += this_round_time;
    mlfq_update_level(&proc_set[curr_proc_idx], proc_set[curr_proc_idx].mlfq_time_running);
    mlfq_reset_level();

    int next_idx = MAX_NPROCESS;
    uint lowest_level = 5;
    for (uint i = 1; i <= MAX_NPROCESS; i++) {
        struct process* p = &proc_set[(curr_proc_idx + i) % MAX_NPROCESS];
        /* Retry the pending system calls of the blocked processes, including
         * the PROC_EXIT message of a process killed by the kernel (a zombie),
         * until GPID_PROCESS receives them. */
        if(p->status == PROC_PENDING_SYSCALL || p->status == PROC_ZOMBIE)
            proc_try_syscall(p);

        // Skip the sleeping process
        clock_now = mtime_get();
        if((p->time_sleep != 0) && (clock_now - p->clock_sleep) < p->time_sleep) {
            continue;
        }
        if(p->time_sleep != 0) p->time_sleep = 0;

        if((p->status == PROC_READY || p->status == PROC_RUNNABLE) &&
            p->mlfq_level < lowest_level) {
            next_idx = (curr_proc_idx + i) % MAX_NPROCESS;
            lowest_level = p->mlfq_level;
        }
    }

    if (next_idx < MAX_NPROCESS) {
        /* [Preemptive Scheduling]
         * Measure and record lifecycle statistics for the *next* process.
         * [System Call & Protection | Multicore & Locks]
         * Modify mstatus.MPP to enter machine or user mode after mret. */
        clock_now = mtime_get();
        if (proc_set[next_idx].clock_switch_in == 0) {
            proc_set[next_idx].clock_response = clock_now;
            proc_set[next_idx].time_response = clock_now - proc_set[next_idx].clock_creation;
        }
        proc_set[next_idx].clock_switch_in = clock_now;

        if(earth->translation == SOFT_TLB && 
           proc_set[next_idx].pid < GPID_USER_START) 
                asm("csrs mstatus, %0" ::"r"(0x1800));
        else    asm("csrc mstatus, %0" ::"r"(0x1800));

    } else {
        /* [Multicore & Locks]
         * Release the kernel lock.
         * [Multicore & Locks | System Call & Protection]
         * Set curr_proc_idx to MAX_NPROCESS; Reset the timer;
         * Enable interrupts by setting the mstatus.MIE bit to 1;
         * Wait for the next interrupt using the wfi instruction. */
        curr_proc_idx = MAX_NPROCESS;
        earth->timer_reset(core_in_kernel);
        asm("csrs mstatus, %0" ::"r"(0x8));
        asm volatile("wfi");
        return;
    }
    /* Student's code ends here. */

    curr_proc_idx = next_idx;
    earth->mmu_switch(curr_pid);
    earth->mmu_flush_cache();
    if (curr_status == PROC_READY) {
        /* Set up the argc, argv, and initial program
         * counter for a newly created process. */
        curr_saved[0]                = APPS_ARG;
        curr_saved[1]                = APPS_ARG + 4;
        proc_set[curr_proc_idx].mepc = APPS_ENTRY;
    }
    proc_set_running(curr_pid);
    earth->timer_reset(core_in_kernel);
}

static void proc_try_send(struct process* sender) {
    for (uint i = 0; i < MAX_NPROCESS; i++) {
        struct process* dst = &proc_set[i];
        if (dst->pid == sender->syscall.receiver &&
            dst->status != PROC_UNUSED) {
            /* Return if process dst is not receiving messages or
             * is not taking messages from the sender process. */
            if (!(dst->syscall.type == SYS_RECV &&
                  dst->syscall.status == PENDING))
                return;
            if (!(dst->syscall.sender == GPID_ALL ||
                  dst->syscall.sender == sender->pid))
                return;

            dst->syscall.status = DONE;
            dst->syscall.sender = sender->pid;
            /* Copy the system call arguments within the kernel PCB. */
            memcpy(dst->syscall.content, sender->syscall.content,
                   SYSCALL_MSG_LEN);
            return;
        }
    }
    FATAL("proc_try_send: unknown receiver pid=%d", sender->syscall.receiver);
}

static void proc_try_recv(struct process* receiver) {
    if (receiver->syscall.status == PENDING) return;

    /* Copy the system call struct from the kernel back to user space. */
    uint syscall_paddr = earth->mmu_translate(receiver->pid, SYSCALL_ARG);
    memcpy((void*)syscall_paddr, &receiver->syscall, sizeof(struct syscall));

    /* Set the receiver and sender back to RUNNABLE. */
    proc_set_runnable(receiver->pid);
    proc_set_runnable(receiver->syscall.sender);
}

static void proc_try_syscall(struct process* proc) {
    switch (proc->syscall.type) {
    case SYS_RECV:
        proc_try_recv(proc);
        break;
    case SYS_SEND:
        proc_try_send(proc);
        break;
    default:
        FATAL("proc_try_syscall: unknown syscall type=%d", proc->syscall.type);
    }
}
