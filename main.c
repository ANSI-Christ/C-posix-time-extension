#define TIME_EXT_IMPL
#include "time_ext.h"

#include <stdio.h>




static void test_timespec(void){
    struct timespec n={10,0};
    struct timespec t1;

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

int main(int argc, char **argv){
    test_timer();
    return 0;
}
