#define TIME_EXT_IMPL
#include "time_ext.h"

#include <stdio.h>



static void test_timespec(void){
    struct timespec t1,t2;

    timespec_current(&t1);
    sleepf(3.3);
    timespec_current(&t2);

    timespec_change(&t2,-t1.tv_sec,-t1.tv_nsec);
    printf("%f ms\n",timespec_milliseconds(&t2));
}



static int timer_callback_1(const char * const s,struct timespec * const t){
    puts(s);
    timespec_change(t,0,500*1000*1000);
    return 1;
}

static int timer_callback_2(const char * const s,struct timespec * const t){
    puts(s);
    return 0;
}

static void my_timer_start(struct timer * const t,const unsigned int delay_ms,void * const f,void * const arg){
    struct timespec x[1];
    timer_init(t,f,arg);
    timespec_current(x);
    timespec_change(x,delay_ms/1000,delay_ms%1000);
    timer_start(t,x,NULL,NULL);
}

static void test_timer(void){
    struct timer t[2];

    my_timer_start(t+0,2000,timer_callback_1,"periodic timer 0.5 sec with delay 2 sec");
    my_timer_start(t+1,5000,timer_callback_2,"single shot timer 5 sec");

    sleepf(10);

    timer_close(t+0);
    timer_close(t+1);
    sleepf(0.1);
}



static void test_gmtime(void){
    struct timespec t;
    struct tm *d;
    time_t x=time(NULL)+3600*3+3600*24*0;

    timespec_runtime(&t,
        unsigned int i=1000000;
        while(i--)
            d=gmtime(&x);
    );

    printf("gmtime %f ms: %d.%d.%d / %d:%d:%d / %d / %d\n",timespec_milliseconds(&t),
    d->tm_mday,d->tm_mon+1,d->tm_year+1900,d->tm_hour,d->tm_min,d->tm_sec,d->tm_wday,d->tm_yday+1);

}

static void test_datetime(void){
    char tmp[64];
    struct timespec t;
    struct datetime d[1];
    time_t x=time(NULL)+3600*3+3600*24*0;

    timespec_runtime(&t,
        unsigned int i=1000000;
        while(i--)
            datetime_from_epoch(d,x);
    );

    printf("dttime %f ms: %s\n",timespec_milliseconds(&t),datetime_string(d,"D.M.Y / h:m:s / w / y",tmp,sizeof(tmp)));
}



int main(int argc, char **argv){
    test_timespec();
    test_gmtime();
    test_datetime();
    test_timer();
    return 0;
}
