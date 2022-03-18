#define _GNU_SOURCE
#include "HPTimer.h"
#include <pthread.h>
#include "stdatomic.h"
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <linux/types.h>
#include <sys/syscall.h>
#include <sched.h>
#include <stdio.h>

struct HPTimer {
    HPTimer_Callback Callback;
    void* Context;
    uint32_t PeriodMs;
    atomic_bool ThreadRunning;
    pthread_t Thread;
};

#define MICROSEC_PER_MS (1000)

struct sched_attr {
    __u32 size;
    __u32 sched_policy;
    __u64 sched_flags;
    /* SCHED_NORMAL, SCHED_BATCH */
    __s32 sched_nice;
    /* SCHED_FIFO, SCHED_RR */
    __u32 sched_priority;
    /* SCHED_DEADLINE (nsec) */
    __u64 sched_runtime;
    __u64 sched_deadline;
    __u64 sched_period;
};

int sched_setattr(pid_t pid, const struct sched_attr* attr,
                  unsigned int flags) {
    return syscall(__NR_sched_setattr, pid, attr, flags);
}

#define NANOSEC_PER_MS (1000000)

static void* ThreadCallback(void* Context) {
    HPTimer* Thread = (HPTimer*)Context;
    struct sched_attr Attr = {0};
    Attr.size = sizeof(Attr);
    Attr.sched_policy = SCHED_DEADLINE;
    Attr.sched_period = Thread->PeriodMs * NANOSEC_PER_MS;
    Attr.sched_deadline = Attr.sched_period / 2;
    Attr.sched_runtime = (Thread->PeriodMs / 3) * NANOSEC_PER_MS;

    int res = sched_setattr(0, &Attr, 0);

    if (res == 0) {
        while (atomic_load(&Thread->ThreadRunning)) {
            sched_yield();
            Thread->Callback(Thread->Context);
        }
    } else {
        uint64_t PeriodUs = Thread->PeriodMs * MICROSEC_PER_MS;
        uint64_t TimeToWait = PeriodUs;
        while (atomic_load(&Thread->ThreadRunning)) {
            usleep(TimeToWait);
            uint64_t ProcStart = HPTimer_GetTimeUs();
            Thread->Callback(Thread->Context);
            uint64_t ProcEnd = HPTimer_GetTimeUs();
            uint64_t ProcTime = ProcEnd - ProcStart;
            if (ProcTime > PeriodUs) {
                TimeToWait = 0;
            } else {
                TimeToWait = PeriodUs - ProcTime;
            }
        }
    }

    return NULL;
}

CommLibStatus COMMLIB_API HPTimer_Initialize(HPTimer_Callback Callback,
                                             void* Context, uint32_t PeriodMs,
                                             HPTimer** Thread) {
    CommLibStatus Status;
    HPTimer* NewThread = malloc(sizeof(HPTimer));
    if (!NewThread) {
        return ENOMEM;
    }
    memset(NewThread, 0, sizeof(*NewThread));

    NewThread->Callback = Callback;
    NewThread->Context = Context;
    NewThread->PeriodMs = PeriodMs;
    atomic_init(&NewThread->ThreadRunning, 1);

    Status =
        pthread_create(&NewThread->Thread, NULL, ThreadCallback, NewThread);
    if (COMMLIB_FAILED(Status)) {
        goto Exit;
    }

    Status = 0;
    *Thread = NewThread;
    NewThread = NULL;

Exit:

    return Status;
}

void COMMLIB_API HPTimer_Free(HPTimer* Thread) {
    atomic_store(&Thread->ThreadRunning, 0);
    pthread_join(Thread->Thread, NULL);
    free(Thread);
}

#define MICROSEC_PER_SEC (1000000)
#define NANOSEC_PER_MICROSEC (1000)

uint64_t COMMLIB_API HPTimer_GetTimeUs(void) {
    struct timespec CurrTime = {0};
    clock_gettime(CLOCK_MONOTONIC, &CurrTime);
    return (CurrTime.tv_sec * MICROSEC_PER_SEC) +
           (CurrTime.tv_nsec / NANOSEC_PER_MICROSEC);
}

void COMMLIB_API HPTimer_SleepMs(uint32_t TimeMs) {
    usleep(TimeMs * 1000);
}