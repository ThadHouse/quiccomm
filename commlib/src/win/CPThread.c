#include "CPThread.h"
#include "Windows.h"
#include <process.h>

struct CPThread {
    CPThread_Main ThreadMain;
    void* Context;
    HANDLE Thread;
    volatile LONG ThreadRunning;
};

static
unsigned int WINAPI CPThreadMain(void* Context) {
    CPThread* Thread = (CPThread*)Context;
    Thread->ThreadMain(Thread->Context);
    return 0;
}

CommLibStatus COMMLIB_API CPThread_Initialize(CPThread_Main ThreadMain, void* Context, CPThread** Thread) {
    CommLibStatus Status;
    DWORD LastError;
    CPThread* NewThread = malloc(sizeof(CPThread));
    if (!NewThread) {
        return E_OUTOFMEMORY;
    }

    RtlZeroMemory(NewThread, sizeof(*NewThread));
    NewThread->ThreadMain = ThreadMain;
    NewThread->Context = Context;
    NewThread->ThreadRunning = 1;

    NewThread->Thread =
        (HANDLE)_beginthreadex(NULL, 0, CPThreadMain, NewThread, 0, NULL);
    if (NewThread->Thread == NULL) {
        LastError = errno;
        Status = HRESULT_FROM_WIN32(LastError);
        goto Exit;
    }

    Status = S_OK;
    *Thread = NewThread;
    NewThread = NULL;

Exit:
    if (NewThread) {
        if (NewThread->Thread != NULL) {
            InterlockedExchange(&NewThread->ThreadRunning, 0);
            WaitForSingleObject(NewThread->Thread, INFINITE);
            CloseHandle(NewThread->Thread);
        }
        free(NewThread);
    }
    return Status;
}

void COMMLIB_API CPThread_Free(CPThread* Thread) {
    InterlockedExchange(&Thread->ThreadRunning, 0);
    WaitForSingleObject(Thread->Thread, INFINITE);
    CloseHandle(Thread->Thread);
    free(Thread);
}