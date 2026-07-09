#include <kernel.h>
#include <scheduler.h>
#include <memory.h>

static task_t tasks[MAX_TASKS];
static int task_count = 0;
static int current_task = -1;

static int task_find_free(void) {
    for (int i = 0; i < MAX_TASKS; i++)
        if (tasks[i].state == TASK_TERMINATED) return i;
    return -1;
}

static int task_find_next_ready(int from) {
    for (int i = 0; i < MAX_TASKS; i++) {
        int idx = (from + i) % MAX_TASKS;
        if (idx < task_count && tasks[idx].state == TASK_READY)
            return idx;
    }
    return -1;
}

int task_create(const char *name, void (*entry)(void)) {
    int idx = task_find_free();
    if (idx < 0) return -1;

    task_t *t = &tasks[idx];
    uint64_t stack_size = 4096;

    uint64_t stack_phys = pmm_alloc();
    if (!stack_phys) return -1;
    memset((void *)(pmm_get_hhdm() + stack_phys), 0, stack_size);
    uint64_t stack_top = pmm_get_hhdm() + stack_phys + stack_size;

    task_regs_t *frame = (task_regs_t *)(stack_top - sizeof(task_regs_t));
    frame->ss = 0x10;
    frame->rsp = stack_top;
    frame->rflags = 0x202;
    frame->cs = 0x08;
    frame->rip = (uint64_t)entry;

    t->rsp = (uint64_t)frame;
    t->kernel_stack_top = stack_top;
    t->kernel_stack_size = stack_size;
    t->state = TASK_READY;

    int i = 0;
    for (; name[i] && i < TASK_NAME_MAX - 1; i++) t->name[i] = name[i];
    t->name[i] = '\0';

    if (idx >= task_count) task_count = idx + 1;
    return idx;
}

void task_exit(void) {
    tasks[current_task].state = TASK_TERMINATED;
    for (;;) scheduler_yield();
}

uint64_t scheduler_tick(uint64_t current_rsp) {
    if (current_task < 0) return 0;
    tasks[current_task].rsp = current_rsp;

    int next = task_find_next_ready(current_task + 1);
    if (next < 0 || next == current_task) {
        tasks[current_task].state = TASK_RUNNING;
        return 0;
    }

    tasks[current_task].state = TASK_READY;
    current_task = next;
    tasks[current_task].state = TASK_RUNNING;
    return tasks[current_task].rsp;
}

void scheduler_yield(void) {
    __asm__ volatile("int $32");
}

void scheduler_init(void) {
    for (int i = 0; i < MAX_TASKS; i++) {
        tasks[i].state = TASK_TERMINATED;
        tasks[i].rsp = 0;
    }

    tasks[0].rsp = 0;
    tasks[0].kernel_stack_top = 0;
    tasks[0].kernel_stack_size = 0;
    tasks[0].state = TASK_RUNNING;
    const char *name = "init";
    for (int i = 0; name[i]; i++) tasks[0].name[i] = name[i];
    tasks[0].name[4] = '\0';
    task_count = 1;
    current_task = 0;
}
