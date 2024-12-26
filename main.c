#define TIME_EXT_IMPL
#include "time_ext.h"

#include <stdio.h>




static void test_timespec(void){
    struct timespec n={10,0};
    struct timespec t1,t2;

    timespec_current(&t1);
    while(nanosleep(&n,&n));
    timespec_current(&t2);

    timespec_change(&t2,-t1.tv_sec,-t1.tv_nsec);
    printf("%f ms\n",timespec_milliseconds(&t2));
}



void timer_callback(const char *s){
    puts(s);
}

static void test_timer(void){
    struct timespec n={10,0};
    struct timer t;

    timer_init(&t,timer_callback,"timer");
    timer_start(&t,2000,500,NULL,NULL);

    while(nanosleep(&n,&n));

    timer_close(&t);
}




static void test_localtime(void){
    struct timespec t;
    struct tm *d;
    time_t x;
    
    timespec_runtime(&t,
        unsigned int i=1000000;
        while(i--){
            x=time(NULL);
            d=gmtime(&x);
        }
    );
    
    printf("gmtime %f ms: %d.%d.%d / %d:%d:%d / %d / %d\n",timespec_milliseconds(&t),
    d->tm_mday,d->tm_mon,d->tm_year,d->tm_hour,d->tm_min,d->tm_sec,d->tm_wday,d->tm_yday);
    
}

static void test_datetime(void){
    char tmp[64];
    struct timespec t;
    struct datetime d[1];
    
    timespec_runtime(&t,
        unsigned int i=1000000;
        while(i--)
            datetime_from_epoch(d,time(NULL));
    );

    printf("datetime %f ms: %s\n",timespec_milliseconds(&t),datetime_string(d,"D.M.Y / h:m:s / w / y",tmp,sizeof(tmp)));
    /*printf("datetime %f ms: %d.%d.%d / %d:%d:%d / %d / %d\n",timespec_milliseconds(&t),
    d->day,d->month,d->year,d->hour,d->minute,d->second,d->day_w,d->day_y);*/
}


int main(int argc, char **argv){
    test_timespec();
    test_localtime();
    test_datetime();
    test_timer();
    return 0;
}
