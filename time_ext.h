/* * * * * * * * * * * * * * * * * */
/* MIT License                     */
/* Copyright (c) 2024 ANSI-Christ  */
/* * * * * * * * * * * * * * * * * */

#ifndef TIME_EXT_H
#define TIME_EXT_H

#include <time.h>
#include <pthread.h>


struct timer {void *_[4]; char __[sizeof(struct timespec)+sizeof(pthread_t)+sizeof(int)*2];};

int timer_init(struct timer *t,void(*callback)(void *arg,unsigned int *interval_ms,pthread_t tid),void *arg);
int timer_start(struct timer *timer,unsigned int delay_ms,unsigned int interval_ms,void(*callback)(void *arg,unsigned int *interval_ms,pthread_t tid),void *arg);

void timer_stop(struct timer *t);
void timer_close(struct timer *t);




int timespec_sign(const struct timespec *t);
int timespec_cmp(const struct timespec *t1,const struct timespec *t2);

void timespec_current(struct timespec *t);
void timespec_normalize(struct timespec *t);
void timespec_runtime(struct timespec *t,...);
void timespec_change(struct timespec *t,long sec,long nanosec);

double timespec_seconds(const struct timespec *t);
double timespec_milliseconds(const struct timespec *t);
double timespec_microseconds(const struct timespec *t);
double timespec_nanoseconds(const struct timespec *t);




extern int nanosleep(const struct timespec*,struct timespec*);
#define timespec_current(_t_) clock_gettime(CLOCK_REALTIME,(_t_))
#define timespec_runtime(_t_,...) do{\
    struct timespec _rt1[1], * const _rt2=(_t_);\
    timespec_current(_rt1); {__VA_ARGS__} timespec_current(_rt2);\
    timespec_change(_rt2,-_rt1->tv_sec,-_rt1->tv_nsec);\
}while(0)

#endif /* TIME_EXT_H */




#ifdef TIME_EXT_IMPL

#include <stdlib.h>

void timespec_normalize(struct timespec * const t){
    t->tv_sec+=t->tv_nsec/1000000000;
    if( (t->tv_nsec%=1000000000)<0){
        t->tv_nsec += 1000000000;
        --t->tv_sec;
    }
}

void timespec_change(struct timespec * const t,const long sec,const long nanosec){
    t->tv_sec+=sec;
    t->tv_nsec+=nanosec;
    timespec_normalize(t);
}

int timespec_sign(const struct timespec * const t){
    const union{float d;int u;}x={timespec_seconds(t)};
    return x.u;
}

int timespec_cmp(const struct timespec * const t1,const struct timespec * const t2){
    const struct timespec t={t1->tv_sec-t2->tv_sec,t1->tv_nsec-t2->tv_nsec};
    return timespec_sign(&t);
}

double timespec_seconds(const struct timespec * const t){
    return t->tv_sec + t->tv_nsec/1000000000.;
}

double timespec_milliseconds(const struct timespec *t){
    return t->tv_sec*1000. + t->tv_nsec/1000000.;
}

double timespec_microseconds(const struct timespec *t){
    return t->tv_sec*1000000. + t->tv_nsec/1000.;
}

double timespec_nanoseconds(const struct timespec *t){
    return t->tv_sec*1000000000 + t->tv_nsec;
}


typedef struct __timer_t{
    struct __timer_t *prev, *next;
    void(*f)(void*,unsigned int*,pthread_t);
    void *arg;
    struct timespec t;
    unsigned int i;
    char run,init;
    pthread_t tid;
} * const _timer_t;

static struct _global_timer_t{
    pthread_mutex_t mtx[1];
    pthread_cond_t cond[1];
    struct __timer_t *first, *last;
    struct timespec t;
}_timer_global={{PTHREAD_MUTEX_INITIALIZER}};

#define TIMER_GLOBAL_LINK(_name_) struct _global_timer_t * const _name_=&_timer_global

static void _timer_link(_timer_t t){
    TIMER_GLOBAL_LINK(g);
    if(g->last){
        g->last->next=t;
        t->prev=g->last;
    }else g->first=t;
    g->last=t;
}

static void _timer_unlink(_timer_t t){
    TIMER_GLOBAL_LINK(g);
    if(t->prev) t->prev->next=t->next;
    else g->first=t->next;
    if(t->next) t->next->prev=t->prev;
    else g->last=t->prev;
}

static void _timer_link_end(_timer_t t){
    TIMER_GLOBAL_LINK(g);
    if(t!=g->last){
        _timer_unlink(t);
        g->last->next=t;
        t->prev=g->last;
        g->last=t;
        t->next=(_timer_t)0;
    }
}

static void _timer_link_beg(_timer_t t){
    TIMER_GLOBAL_LINK(g);
    if(t!=g->first){
        _timer_unlink(t);
        g->first->prev=t;
        t->next=g->first;
        g->first=t;
        t->prev=(_timer_t)0;
    }
}

static void _timer_sched(void){
    const pthread_t tid=pthread_self();
    struct sched_param pri; int pol;
    if(!pthread_getschedparam(tid,&pol,&pri)){
        pri.sched_priority=99;
        pthread_setschedparam(tid,pol,&pri);
    }
}

static void *_timer_thread(void *arg){
    TIMER_GLOBAL_LINK(g);
    struct timespec t;
    struct __timer_t *timer;
    _timer_sched();

    while('0'){
        pthread_mutex_lock(g->mtx);
_mark:
        t=g->t;
        switch(pthread_cond_timedwait(g->cond,g->mtx,&t)){
            case -1: if(errno==EINTR) goto _mark; break;
            case 0: if(g->first) goto _mark; break;
            case EINTR: goto _mark;
        }
        if(!(timer=g->first)){
            g->t.tv_sec=0;
            pthread_cond_destroy(g->cond);
            pthread_mutex_unlock(g->mtx);
            break;
        }
        timespec_current(&t);
        g->t.tv_sec=t.tv_sec+3600;
        while(timer && timer->run){
            if(timespec_cmp(&timer->t,&t)<1){
                timer->f(timer->arg,&timer->i,timer->tid);
                if(!timer->i){
                    timer_t next=timer->next;
                    timer->run=0;
                    _timer_link_end(timer);
                    timer=next;
                    continue;
                }
                timer->t=t;
                timespec_change(&timer->t,timer->i/1000,(timer->i%1000)*1000000);
            }
            if(timespec_cmp(&timer->t,&g->t)<0)
                g->t=timer->t;
            timer=timer->next;
        }
        pthread_mutex_unlock(g->mtx);
    }
    return NULL;
    (void)arg;
}

static char _timer_init(void){
    TIMER_GLOBAL_LINK(g);
    if(!g->t.tv_sec){
        pthread_attr_t a[1];
        pthread_t tid[1];
        do{
            if(pthread_attr_init(a)) break;
            if(pthread_cond_init(g->cond,NULL)) break;
            if(pthread_attr_setstacksize(a,20<<10)) break;
            if(pthread_attr_setdetachstate(a,PTHREAD_CREATE_DETACHED)) break;
            if(pthread_attr_setinheritsched(a,PTHREAD_INHERIT_SCHED)) break;
            #ifdef PTHREAD_FPU_ENABLED
            if(pthread_setfpustate(a,PTHREAD_FPU_ENABLED)) break;
            #endif
            if(!pthread_create(tid,a,_timer_thread,NULL))
                g->t.tv_sec=time(NULL)+3600;
        }while(0);
        pthread_attr_destroy(a);
        if(!g->t.tv_sec) pthread_cond_destroy(g->cond);
    } return g->t.tv_sec>0;
}

int timer_init(struct timer *timer,void(*callback)(void *arg,unsigned int *interval_ms,pthread_t tid),void *arg){
    _timer_t t=(_timer_t)timer;
    TIMER_GLOBAL_LINK(g);
    t->prev=t->next=(_timer_t)0;
    t->f=callback;
    t->arg=arg;
    t->run=t->init=0;
    pthread_mutex_lock(g->mtx);
    if(_timer_init()){
        _timer_link(t);
        t->init=1;
    }
    pthread_mutex_unlock(g->mtx);
    return t->init-1;
}

int timer_start(struct timer *timer,unsigned int delay_ms,unsigned int interval_ms,void(*callback)(void *arg,unsigned int *interval_ms,pthread_t tid),void *arg){
    _timer_t t=(_timer_t)timer;
    if(t->init && (delay_ms|interval_ms) && (callback || t->f)){
        TIMER_GLOBAL_LINK(g);
        const pthread_t tid=pthread_self();
        pthread_mutex_lock(g->mtx);
        if(arg) t->arg=arg;
        if(callback) t->f=(void*)callback;
        if(!t->run){
            t->run=1;
            _timer_link_beg(t);
        }
        t->tid=tid;
        t->i=interval_ms;
        timespec_current(&t->t);
        timespec_change(&t->t,delay_ms/1000,(delay_ms%1000)*1000000);
        if(timespec_cmp(&t->t,&g->t)<0){
            g->t=t->t;
            pthread_cond_signal(g->cond);
        }
        pthread_mutex_unlock(g->mtx);
        return 0;
    } return -1;
}

void timer_stop(struct timer *timer){
    _timer_t t=(_timer_t)timer;
    if(t->init && t->run){
        TIMER_GLOBAL_LINK(g);
        pthread_mutex_lock(g->mtx);
        t->run=0;
        _timer_link_end(t);
        pthread_mutex_unlock(g->mtx);
    }
}


int timer_continue(struct timer *timer){
    _timer_t t=(_timer_t)timer;
    if(t->init && !t->run && t->f){
        TIMER_GLOBAL_LINK(g);
        pthread_mutex_lock(g->mtx);
        t->run=1;
        _timer_link_beg(t);
        if(timespec_cmp(&t->t,&g->t)<0){
            g->t=t->t;
            pthread_cond_signal(g->cond);
        }
        pthread_mutex_unlock(g->mtx);
        return 0;
    } return -1;
}

void timer_close(struct timer *timer){
    _timer_t t=(_timer_t)timer;
    if(t->init){
        TIMER_GLOBAL_LINK(g);
        _timer_t t=(_timer_t)timer;
        pthread_mutex_lock(g->mtx);
        _timer_unlink(t);
        if(!g->first) pthread_cond_signal(g->cond);
        pthread_mutex_unlock(g->mtx);
        t->init=0;
    }
}

#undef TIMER_GLOBAL_LINK

#endif