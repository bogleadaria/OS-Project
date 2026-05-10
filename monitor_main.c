#include "monitor_reports.h"

#include <stdio.h>
#include <unistd.h>

int main(){
    write_pid_file();

    printf("[monitor] Started, PID = %d\n", (int)getpid());

    setup_signals();

    while(1){
        pause();
    }

    return 0;
}