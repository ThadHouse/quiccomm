#include "CPThread.h"
#include <pthread.h>
#include <stdatomic.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct CPThread {
    CPThread_Main ThreadMain;
    void* Context;
    atomic_bool ThreadRunning;
    pthread_t Thread;
};

static void* CPThreadMain(void* Context) {
    CPThread* Thread = (CPThread*)Context;
    Thread->ThreadMain(Thread->Context);
    return NULL;
}

CommLibStatus COMMLIB_API CPThread_Initialize(CPThread_Main ThreadMain,
                                              void* Context,
                                              CPThread** Thread) {
    CommLibStatus Status;
    CPThread* NewThread = malloc(sizeof(CPThread));
    if (!NewThread) {
        return ENOMEM;
    }
    memset(NewThread, 0, sizeof(*NewThread));

    NewThread->ThreadMain = ThreadMain;
    NewThread->Context = Context;
    atomic_init(&NewThread->ThreadRunning, 1);

    Status = pthread_create(&NewThread->Thread, NULL, CPThreadMain, NewThread);
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

void COMMLIB_API CPThread_Free(CPThread* Thread) {
    atomic_store(&Thread->ThreadRunning, 0);
    pthread_join(Thread->Thread, NULL);
    free(Thread);
}