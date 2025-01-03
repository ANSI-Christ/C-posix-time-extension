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



void timer_callback(const char *s){
    puts(s);
}

static void test_timer(void){
    struct timer t;

    timer_init(&t,timer_callback,"timer");
    timer_start(&t,2000,500,NULL,NULL);

    sleepf(10);

    timer_close(&t);
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
