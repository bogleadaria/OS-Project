#define _POSIX_C_SOURCE 200809L

#include "monitor_reports.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

static void handle_sigusr1(int sig)
{
    (void)sig;
    const char *msg = "[monitor] New report added.\n";
    write(STDOUT_FILENO, msg, strlen(msg));
}

static void handle_sigint(int sig)
{
    (void)sig;
    unlink(PID_FILE);
    const char *msg = "[monitor] SIGINT received. Shutting down.\n";
    write(STDOUT_FILENO, msg, strlen(msg));
}

void write_pid_file()
{
    int f = open(PID_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (f == -1)
    {
        perror("Failed to create .monitor_pid");
        _exit(1);
    }
    char pid_str[32];
    int len = snprintf(pid_str, sizeof(pid_str), "%d\n", (int)getpid());

    write(f, pid_str, len);

    close(f);
}

void setup_signals()
{
    struct sigaction sa_usr1;
    memset(&sa_usr1, 0, sizeof(sa_usr1));
    sa_usr1.sa_handler = handle_sigusr1;
    sigemptyset(&sa_usr1.sa_mask);
    sa_usr1.sa_flags = SA_RESTART;
    if (sigaction(SIGUSR1, &sa_usr1, NULL) == -1)
    {
        perror("sigaction SIGUSR1");
        _exit(1);
    }
    struct sigaction sa_int;
    memset(&sa_int, 0, sizeof(sa_int));
    sa_int.sa_handler = handle_sigint;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0;
    if (sigaction(SIGINT, &sa_int, NULL) == -1)
    {
        perror("sigaction SIGINT");
        _exit(1);
    }
}