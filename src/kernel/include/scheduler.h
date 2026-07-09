#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>

#define TASK_NAME_MAX 32
#define MAX_TASKS     64

typedef enum {
    TASK_READY = 0,
    TASK_RUNNING,
    TASK_BLOCKED,
    TASK_TERMINATED
} task_state_t;

typedef struct {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t int_no;
    uint64_t error_code;
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
} __attribute__((packed)) task_regs_t;

typedef struct task {
    uint64_t      rsp;
    uint64_t      kernel_stack_top;
    uint64_t      kernel_stack_size;
    task_state_t  state;
    char          name[TASK_NAME_MAX];
} task_t;

void scheduler_init(void);
int  task_create(const char *name, void (*entry)(void));
void task_exit(void);
void scheduler_yield(void);
uint64_t scheduler_tick(uint64_t current_rsp);

#endif
