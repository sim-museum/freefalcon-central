/* FreeFalcon Linux Port - process.h compatibility */
#ifndef FF_COMPAT_PROCESS_H
#define FF_COMPAT_PROCESS_H
#ifdef FF_LINUX

#include "compat_types.h"
#include "compat_winbase.h"
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void (*start)(void *);
    void *arg;
} FF_BEGINTHREAD_ARGS;

static inline void *FF_BeginThreadTrampoline(void *p) {
    FF_BEGINTHREAD_ARGS a = *(FF_BEGINTHREAD_ARGS *)p;
    free(p);
    a.start(a.arg);
    return NULL;
}

static inline uintptr_t _beginthread(void (*start_address)(void *), unsigned stack_size, void *arglist) {
    (void)stack_size;
    pthread_t thread;
    FF_BEGINTHREAD_ARGS *ta = (FF_BEGINTHREAD_ARGS *)malloc(sizeof(FF_BEGINTHREAD_ARGS));
    ta->start = start_address;
    ta->arg = arglist;
    if (pthread_create(&thread, NULL, FF_BeginThreadTrampoline, ta) != 0) {
        free(ta);
        return (uintptr_t)-1;
    }
    pthread_detach(thread);
    return (uintptr_t)thread;
}

typedef struct {
    unsigned (*start)(void *);
    void *arg;
} FF_BEGINTHREADEX_ARGS;

static inline void *FF_BeginThreadExTrampoline(void *p) {
    FF_BEGINTHREADEX_ARGS a = *(FF_BEGINTHREADEX_ARGS *)p;
    free(p);
    a.start(a.arg);
    return NULL;
}

/* Returns a tagged FF_DETACHED_THREAD_HANDLE so CloseHandle can free it
 * (see compat_winbase.h). Port docs Problem 5. */
static inline uintptr_t _beginthreadex(void *security, unsigned stack_size,
                                       unsigned (*start_address)(void *),
                                       void *arglist, unsigned initflag, unsigned *thrdaddr) {
    (void)security; (void)stack_size; (void)initflag;
    FF_DETACHED_THREAD_HANDLE *h = (FF_DETACHED_THREAD_HANDLE *)malloc(sizeof(FF_DETACHED_THREAD_HANDLE));
    if (!h) return 0;
    h->type = FF_HANDLE_TYPE_DETACHED_THREAD;
    FF_BEGINTHREADEX_ARGS *ta = (FF_BEGINTHREADEX_ARGS *)malloc(sizeof(FF_BEGINTHREADEX_ARGS));
    ta->start = start_address;
    ta->arg = arglist;
    if (pthread_create(&h->thread, NULL, FF_BeginThreadExTrampoline, ta) != 0) {
        free(ta);
        free(h);
        return 0;
    }
    pthread_detach(h->thread);
    if (thrdaddr) *thrdaddr = (unsigned)(uintptr_t)h->thread;
    return (uintptr_t)h;
}

static inline void _endthread(void) { pthread_exit(NULL); }
static inline void _endthreadex(unsigned retval) { (void)retval; pthread_exit(NULL); }

#define _getpid getpid

#ifdef __cplusplus
}
#endif

#endif /* FF_LINUX */
#endif /* FF_COMPAT_PROCESS_H */
