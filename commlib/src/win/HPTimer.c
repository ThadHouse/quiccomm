#include "HPTimer.h"
#include "Windows.h"

struct HPTimer {
    HPTimer_Callback Callback;
    void* Context;
    uint32_t PeriodMs;
    HANDLE Thread;
    volatile LONG ThreadRunning;
    HANDLE ReadyEvent;
};

static uint64_t PerfFreq;

//
// Converts microseconds to platform time.
//
static LARGE_INTEGER TimeUs64ToPlat(uint64_t TimeUs) {
    uint64_t High = (TimeUs >> 32) * PerfFreq;
    uint64_t Low = (TimeUs & 0xFFFFFFFF) * PerfFreq;
    LARGE_INTEGER Ret = {0};
    Ret.QuadPart = ((High / 1000000) << 32) +
                   ((Low + ((High % 1000000) << 32)) / PerfFreq);
    return Ret;
}

static DWORD WINAPI ThreadMain(LPVOID Parameter) {
    DWORD Status = 0;
    HPTimer* Thread = (HPTimer*)Parameter;
    HANDLE Timer = CreateWaitableTimerA(NULL, FALSE, NULL);
    if (Timer == NULL) {
        Status = GetLastError();
        goto Exit;
    }

    LARGE_INTEGER DueTime;
    DueTime.QuadPart = -(int32_t)Thread->PeriodMs;
    if (!SetWaitableTimer(Timer, &DueTime, Thread->PeriodMs, NULL, NULL, 0)) {
        Status = GetLastError();
        goto Exit;
    }
    SetEvent(Thread->ReadyEvent);
    while (Thread->ThreadRunning) {
        WaitForSingleObject(Timer, INFINITE);
        Thread->Callback(Thread->Context);
    }

Exit:
    if (Timer != NULL) {
        CloseHandle(Timer);
    }
    InterlockedExchange(&Thread->ThreadRunning, 0);
    SetEvent(Thread->ReadyEvent);
    return Status;
}

CommLibStatus COMMLIB_API HPTimer_Initialize(HPTimer_Callback Callback,
                                             void* Context, uint32_t PeriodMs,
                                             HPTimer** Thread) {
    if (PerfFreq == 0) {
        LARGE_INTEGER Freq = {0};
        if (!QueryPerformanceFrequency(&Freq)) {
            DWORD LastError = GetLastError();
            return HRESULT_FROM_WIN32(LastError);
        }
        PerfFreq = Freq.QuadPart;
    }

    CommLibStatus Status;
    DWORD LastError;
    HPTimer* NewThread = malloc(sizeof(HPTimer));
    if (!NewThread) {
        return E_OUTOFMEMORY;
    }

    RtlZeroMemory(NewThread, sizeof(*NewThread));
    NewThread->Callback = Callback;
    NewThread->Context = Context;
    NewThread->PeriodMs = PeriodMs;

    NewThread->ThreadRunning = 1;

    NewThread->ReadyEvent = CreateEventA(NULL, TRUE, FALSE, NULL);
    if (NewThread->ReadyEvent == NULL) {
        LastError = GetLastError();
        Status = HRESULT_FROM_WIN32(LastError);
        goto Exit;
    }

    NewThread->Thread = CreateThread(NULL, 0, ThreadMain, NewThread, 0, NULL);
    if (NewThread->Thread == NULL) {
        LastError = GetLastError();
        Status = HRESULT_FROM_WIN32(LastError);
        goto Exit;
    }

    if (!SetThreadPriority(NewThread->Thread, THREAD_PRIORITY_HIGHEST)) {
        LastError = GetLastError();
        Status = HRESULT_FROM_WIN32(LastError);
        goto Exit;
    }

    WaitForSingleObject(NewThread->ReadyEvent, INFINITE);
    if (!NewThread->ThreadRunning) {
        WaitForSingleObject(NewThread->Thread, INFINITE);
        GetExitCodeThread(NewThread->Thread, &LastError);
        Status = HRESULT_FROM_WIN32(LastError);
        CloseHandle(NewThread->Thread);
        NewThread->Thread = NULL;
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
        if (NewThread->ReadyEvent) {
            CloseHandle(NewThread->ReadyEvent);
        }
        free(Thread);
    }
    return Status;
}

void COMMLIB_API HPTimer_Free(HPTimer* Thread) {
    InterlockedExchange(&Thread->ThreadRunning, 0);
    WaitForSingleObject(Thread->Thread, INFINITE);
    CloseHandle(Thread->Thread);
    CloseHandle(Thread->ReadyEvent);
    free(Thread);
}

//
// Converts platform time to microseconds.
//
static uint64_t TimePlatToUs64(LARGE_INTEGER Count) {
    //
    // Multiply by a big number (1000000, to convert seconds to microseconds)
    // and divide by a big number (CxPlatPerfFreq, to convert counts to secs).
    //
    // Avoid overflow with separate multiplication/division of the high and low
    // bits. Taken from TcpConvertPerformanceCounterToMicroseconds.
    //
    uint64_t High = (Count.QuadPart >> 32) * 1000000;
    uint64_t Low = (Count.QuadPart & 0xFFFFFFFF) * 1000000;
    return ((High / PerfFreq) << 32) +
           ((Low + ((High % PerfFreq) << 32)) / PerfFreq);
}

uint64_t COMMLIB_API HPTimer_GetTimeUs(void) {
    LARGE_INTEGER Count = {0};
    if (!QueryPerformanceCounter(&Count)) {
        return 0;
    }
    return TimePlatToUs64(Count);
}

void COMMLIB_API HPTimer_SleepMs(uint32_t TimeMs) {
    Sleep(TimeMs);
}