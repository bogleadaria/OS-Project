#include "monitor_reports.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


static void handle_sigusr1(int sig){
    (void)sig;
    const char *msg = "[monitor] New report added.\n";
    write(STDOUT_FILENO, msg, strlen(msg));
}

static void handle_sigint(int sig){
    (void)sig;
    unlink(PID_FILE);
    const char *msg="[monitor] SIGINT received. Shutting down.\n";
    write(STDOUT_FILENO, msg, strlen(msg));
}