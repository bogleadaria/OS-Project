#define _POSIX_C_SOURCE 200809L

#include "monitor_reports.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    int pipe_fd = -1;

    if (argc >= 2)
    {
        pipe_fd = atoi(argv[1]);
        if (pipe_fd <= 0)
        {
            pipe_fd = -1;
        }
    }

    if (check_existing_monitor(pipe_fd))
    {
        if (pipe_fd != -1)
        {
            close(pipe_fd);
        }
        return 1;
    }

    write_pid_file();

    char msg[64];
    snprintf(msg, sizeof(msg), "Started, PID = %d", (int)getpid());
    char buf[128];
    int len = snprintf(buf, sizeof(buf), "%s%s\n", MSG_INFO, msg);
    if (pipe_fd != -1)
    {
        write(pipe_fd, buf, len);
    }
    else
    {
        write(1, buf, len);
    }

    setup_signals(pipe_fd);

    while (1)
    {
        pause();
    }

    return 0;
}