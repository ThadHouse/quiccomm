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

int mtx_init(mtx_t* mutex, int type) {
    *mutex = NULL;

    InitializeCriticalSection()
}

int mtx_lock(mtx_t* mutex);

int mtx_timedlock(mtx_t* mutex,
                  const struct timespec* time_point);

int mtx_trylock(mtx_t* mutex);

int mtx_unlock(mtx_t* mutex);

void mtx_destroy(mtx_t* mutex);

int cnd_init(cnd_t* cond);

int cnd_signal(cnd_t* cond);

int cnd_broadcast(cnd_t* cond);

int cnd_wait(cnd_t* cond, mtx_t* mutex);

int cnd_timedwait(cnd_t* cont, mtx_t* mutex,
                  const struct timespec* time_point);

void cnd_destroy(cnd_t* cond);