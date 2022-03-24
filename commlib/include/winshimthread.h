#pragma once

#include "process.h"
#include "time.h"
#include <stdlib.h>

typedef void* thrd_t;

enum {
    thrd_success = 0,
    thrd_nomem = 1,
    thrd_timedout = 2,
    thrd_busy = 3,
    thrd_error = 4,
};

typedef int (*thrd_start_t)(void* arg);

int thrd_create(thrd_t* thr, thrd_start_t func, void* arg);

int thrd_equal(thrd_t lhs, thrd_t rhs);

thrd_t thrd_current(void);

struct timespec;

int thrd_sleep(const struct timespec* duration, struct timespec* remaining);

void thrd_yield(void);

void thrd_exit(int res);

int thrd_detach(thrd_t thr);

int thrd_join(thrd_t thr, int* res);

/* Mutex */

typedef int mtx_t;

enum { mtx_plain = 0, mtx_recursive = 1, mtx_timed = 2 };

int mtx_init(mtx_t* mutex, int type);

int mtx_lock(mtx_t* mutex);

struct timespec;

int mtx_timedlock(mtx_t* mutex,
                  const struct timespec* time_point);

int mtx_trylock(mtx_t* mutex);

int mtx_unlock(mtx_t* mutex);

void mtx_destroy(mtx_t* mutex);

typedef int cnd_t;

int cnd_init(cnd_t* cond);

int cnd_signal(cnd_t* cond);

int cnd_broadcast(cnd_t* cond);

int cnd_wait(cnd_t* cond, mtx_t* mutex);

int cnd_timedwait(cnd_t* cont, mtx_t* mutex,
                  const struct timespec* time_point);

void cnd_destroy(cnd_t* cond);