#include "HPTimer.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <mach/mach.h>
#include <mach/mach_time.h>
#include <pthread.h>
#include "stdatomic.h"

static int move_pthread_to_realtime_scheduling_class(pthread_t pthread,
                                                     uint32_t timems) {
    mach_timebase_info_data_t timebase_info;
    mach_timebase_info(&timebase_info);

    const uint64_t NANOS_PER_MSEC = 1000000ULL;
    double clock2abs =
        ((double)timebase_info.denom / (double)timebase_info.numer) *
        NANOS_PER_MSEC;

    thread_time_constraint_policy_data_t policy;
    policy.period = timems * clock2abs;
    policy.computation = (timems / 3) * clock2abs;
    policy.constraint = (timems / 2) * clock2abs;
    policy.preemptible = TRUE;

    int kr = thread_policy_set(
        pthread_mach_thread_np(pthread), THREAD_TIME_CONSTRAINT_POLICY,
        (thread_policy_t)&policy, THREAD_TIME_CONSTRAINT_POLICY_COUNT);
    if (kr != KERN_SUCCESS) {
        mach_error("thread_policy_set:", kr);
        return kr;
    }
    return KERN_SUCCESS;
}

#define NANOS_PER_USEC (1000ULL)
#define NANOS_PER_MILLISEC (1000ULL * NANOS_PER_USEC)

struct HPTimer {
    HPTimer_Callback Callback;
    void* Context;
    uint32_t PeriodMs;
    atomic_bool ThreadRunning;
    pthread_t Thread;
};

static void* ThreadCallback(void* Context) {
    HPTimer* Thread = (HPTimer*)Context;
    mach_timebase_info_data_t timebase_info;
    uint64_t period_nanos = Thread->PeriodMs * NANOS_PER_MILLISEC;
    mach_timebase_info(&timebase_info);
    uint64_t period = period_nanos * timebase_info.denom / timebase_info.numer;
    uint64_t time_to_wait = period;
    while (atomic_load(&Thread->ThreadRunning)) {
        uint64_t next_wake = mach_absolute_time() + time_to_wait;
        mach_wait_until(next_wake);
        uint64_t proc_start = mach_absolute_time();
        Thread->Callback(Thread->Context);
        uint64_t proc_end = mach_absolute_time();
        uint64_t proc_time = proc_end - proc_start;
        if (proc_time > period) {
            time_to_wait = 0;
        } else {
            time_to_wait = period - proc_time;
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

    Status =
        move_pthread_to_realtime_scheduling_class(NewThread->Thread, PeriodMs);
    if (COMMLIB_FAILED(Status)) {
        goto Exit;
    }

    Status = 0;
    *Thread = NewThread;
    NewThread = NULL;

Exit:
    if (NewThread) {
        atomic_store(&NewThread->ThreadRunning, 0);
        if (NewThread->Thread) {
            pthread_join(NewThread->Thread, NULL);
        }
        free(NewThread);
    }
    return Status;
}

void COMMLIB_API HPTimer_Free(HPTimer* Thread) {
    atomic_store(&Thread->ThreadRunning, 0);
    pthread_join(Thread->Thread, NULL);
    free(Thread);
}

uint64_t COMMLIB_API HPTimer_GetTimeUs(void) {
    uint64_t nanos = clock_gettime_nsec_np(CLOCK_UPTIME_RAW);
    return nanos / NANOS_PER_USEC;
}
