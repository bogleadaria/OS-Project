#define _POSIX_C_SOURCE 200809L

#include "monitor_reports.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

static int g_pipe_fd = -1;

static void pipe_write(const char *prefix, const char *msg)
{
    char buf[512];
    int len = snprintf(buf, sizeof(buf), "%s%s\n", prefix, msg);
    if (len <= 0)
    {
        return;
    }
    if (g_pipe_fd != -1)
    {
        write(g_pipe_fd, buf, len);
    }
    else
    {
        write(STDOUT_FILENO, buf, len);
    }
}

static void handle_sigusr1(int sig)
{
    (void)sig;
    pipe_write(MSG_INFO, "New report added.");
}

static void handle_sigint(int sig)
{
    (void)sig;
    unlink(PID_FILE);
    pipe_write(MSG_EXIT, "SIGINT received. Shutting down.");
    if (g_pipe_fd != -1)
    {
        close(g_pipe_fd);
    }
    _exit(0);
}

int check_existing_monitor(int pipe_fd)
{
    int f = open(PID_FILE, O_RDONLY);
    if (f == -1)
    {
        return 0;
    }

    char pid_str[32];
    memset(pid_str, 0, sizeof(pid_str));
    ssize_t n = read(f, pid_str, sizeof(pid_str) - 1);
    close(f);
    if (n <= 0)
    {
        return 0;
    }

    pid_t existing_pid = (pid_t)atoi(pid_str);
    if (existing_pid <= 0)
    {
        return 0;
    }

    if (kill(existing_pid, 0) == -1)
    {
        return 0;
    }

    char msg[128];
    snprintf(msg, sizeof(msg), "Monitor already running with PID %d.", (int)existing_pid);

    char buf[256];
    int len = snprintf(buf, sizeof(buf), "%s%s\n", MSG_ERROR, msg);
    
    if (pipe_fd != -1)
    {
        write(pipe_fd, buf, len);
    }
    else
    {
        write(STDOUT_FILENO, buf, len);
    }
    return 1;
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
    int len = snprintf(pid_str, sizeof(pid_str), "%d\n", (int)getpid());
    write(f, pid_str, len);
    close(f);
}

void setup_signals(int pipe_fd)
{
    g_pipe_fd = pipe_fd;

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