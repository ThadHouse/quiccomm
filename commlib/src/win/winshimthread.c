#include "winshimthread.h"
#ifdef _WIN32
#pragma warning(disable : 5105)
#endif
#include <Windows.h>

// https://github.com/microsoft/STL/blob/18d8f37c990f06af93820afdfa16388ff3d21db1/stl/src/cthread.cpp
// https://github.com/microsoft/STL/blob/62137922ab168f8e23ec1a95c946821e24bde230/stl/src/cond.cpp
// https://github.com/microsoft/STL/blob/62137922ab168f8e23ec1a95c946821e24bde230/stl/src/mutex.cpp

typedef struct ThreadStorage {
    thrd_start_t func;
    void* arg;
} ThreadStorage;

static unsigned __stdcall ThreadCallback(void* arg) {
    ThreadStorage* Storage = (ThreadStorage*)arg;
    int ret = Storage->func(Storage->arg);
    free(Storage);
    return (unsigned)ret;
}

int thrd_create(thrd_t* thr, thrd_start_t func, void* arg) {
    ThreadStorage* storage = malloc(sizeof(ThreadStorage));
    if (storage == NULL) {
        return thrd_nomem;
    }
    storage->arg = arg;
    storage->func = func;
    *thr = (void*)_beginthreadex(NULL, 0, ThreadCallback, storage, 0, NULL);
    if (*thr == NULL) {
        free(storage);
        return thrd_error;
    }
    return thrd_success;
}

int thrd_equal(thrd_t lhs, thrd_t rhs) {
    return GetThreadId(lhs) == GetThreadId(rhs);
}

thrd_t thrd_current(void) {
    return GetCurrentThread();
}

struct timespec;

int thrd_sleep(const struct timespec* duration, struct timespec* remaining);

void thrd_yield(void) {
    SwitchToThread();
}

void thrd_exit(int res) {
    _endthreadex(res);
}

int thrd_detach(thrd_t thr) {
    return CloseHandle(thr) == 0 ? thrd_error : thrd_success;
}

int thrd_join(thrd_t thr, int* res) {
    DWORD resu;
    if (WaitForSingleObjectEx(thr, INFINITE, FALSE) == WAIT_FAILED || GetExitCodeThread(thr, &resu) == 0) {
        return thrd_error;
    }

    if (res) {
        *res = (int)resu;
    }

    return CloseHandle(thr) == 0 ? thrd_error : thrd_success;
}

/* Mutex */

typedef struct mtx_impl {
    int Type;
    union {
        CRITICAL_SECTION CS;
        SRWLOCK SRW;
    };
} mtx_impl;

int mtx_init(mtx_t* mutex, int type) {
    *mutex = NULL;

    if (type != mtx_plain && type != mtx_recursive) {
        return thrd_error;
    }

    mtx_impl* Impl = malloc(sizeof(mtx_impl));

    if (!Impl) {
        return thrd_nomem;
    }

    RtlZeroMemory(Impl, sizeof(*Impl));

    Impl->Type = type;

    if (type == mtx_plain) {
        InitializeSRWLock(&Impl->SRW);
    } else {
        InitializeCriticalSection(&Impl->CS);
    }

    *mutex = Impl;
    return thrd_success;
}

int mtx_lock(mtx_t* mutex) {
    mtx_impl* Impl = (mtx_impl*)*mutex;

    if (Impl->Type == mtx_plain) {
        AcquireSRWLockExclusive(&Impl->SRW);
    } else {
        EnterCriticalSection(&Impl->CS);
    }

    return thrd_success;
}

// int mtx_timedlock(mtx_t* mutex,
//                   const struct timespec* time_point) {
//                       return thrd_error;
//                   }

int mtx_trylock(mtx_t* mutex) {
    mtx_impl* Impl = (mtx_impl*)*mutex;

    BOOL Ret;
    if (Impl->Type == mtx_plain) {
        Ret = TryAcquireSRWLockExclusive(&Impl->SRW);
    } else {
        Ret = TryEnterCriticalSection(&Impl->CS);
    }
    return Ret ? thrd_success : thrd_busy;
}

int mtx_unlock(mtx_t* mutex) {
    mtx_impl* Impl = (mtx_impl*)*mutex;

    if (Impl->Type == mtx_plain) {
        ReleaseSRWLockExclusive(&Impl->SRW);
    } else {
        LeaveCriticalSection(&Impl->CS);
    }

    return thrd_success;
}

void mtx_destroy(mtx_t* mutex) {
    mtx_impl* Impl = (mtx_impl*)*mutex;

    if (Impl->Type == mtx_recursive) {
        DeleteCriticalSection(&Impl->CS);
    }
    free(Impl);
}

typedef struct cnd_impl {
    CONDITION_VARIABLE CV;
} cnd_impl;

int cnd_init(cnd_t* cond) {
    *cond = NULL;

    cnd_impl* Impl = malloc(sizeof(cnd_impl));

    if (!Impl) {
        return thrd_nomem;
    }

    RtlZeroMemory(Impl, sizeof(*Impl));

    InitializeConditionVariable(&Impl->CV);

    *cond = Impl;
    return thrd_success;
}

int cnd_signal(cnd_t* cond) {
    cnd_impl* Impl = (cnd_impl*)*cond;

    WakeConditionVariable(&Impl->CV);

    return thrd_success;
}

int cnd_broadcast(cnd_t* cond) {
    cnd_impl* Impl = (cnd_impl*)*cond;

    WakeAllConditionVariable(&Impl->CV);

    return thrd_success;
}

int cnd_wait(cnd_t* cond, mtx_t* mutex) {
    cnd_impl* Impl = (cnd_impl*)*cond;
    mtx_impl* MtxImpl = (mtx_impl*)*mutex;

    BOOL Ret;
    if (MtxImpl->Type == mtx_plain) {
        Ret = SleepConditionVariableSRW(&Impl->CV, &MtxImpl->SRW, INFINITE, 0);
    } else {
        Ret = SleepConditionVariableCS(&Impl->CV, &MtxImpl->CS, INFINITE);
    }
    return Ret ? thrd_success : thrd_error;
}

// int cnd_timedwait(cnd_t* cont, mtx_t* mutex,
//                   const struct timespec* time_point);

void cnd_destroy(cnd_t* cond) {
    cnd_impl* Impl = (cnd_impl*)*cond;

    free(Impl);
}
