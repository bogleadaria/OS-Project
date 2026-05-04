#include "monitor_reports.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

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

void write_pid_file(void)
{
    int f = open(PID_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (f == -1)
    {
        perror("Failed to create .monitor_pid");
        _exit(1);
    }
    char pid_str[32];
    int len = snprintf(pid_str, "%d\n", (int)getpid);

    write(f, pid_str, len);

    close(f);
}