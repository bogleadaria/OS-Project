#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>

#include "report.h"

#define MONITOR_BIN "./monitor"
#define SCORER_BIN  "./scorer"

static void hub_mon_loop(int read_fd)
{
    char buf[512];
    char line[512];
    int  pos = 0;

    while (1)
    {
        ssize_t n = read(read_fd, buf, sizeof(buf));
        if (n <= 0)
        {
            printf("[hub_mon] Monitor pipe closed. Monitor has ended.\n");
            fflush(stdout);
            break;
        }

        for (ssize_t i = 0; i < n; i++)
        {
            if (buf[i] == '\n')
            {
                line[pos] = '\0';
                pos = 0;

                if (strncmp(line, "INFO:", 5) == 0)
                {
                    printf("[monitor] %s\n", line + 5);
                }
                else if (strncmp(line, "ERROR:", 6) == 0)
                {
                    printf("[monitor] ERROR: %s\n", line + 6);
                }
                else if (strncmp(line, "EXIT:", 5) == 0)
                {
                    printf("[monitor] %s\n", line + 5);
                    printf("[hub_mon] Monitor has ended.\n");
                }
                else
                {
                    printf("[monitor] %s\n", line);
                }
                fflush(stdout);
            }
            else
            {
                if (pos < (int)sizeof(line) - 1)
                    line[pos++] = buf[i];
            }
        }
    }
}

static void cmd_start_monitor(void)
{
    int pipefd[2];
    if (pipe(pipefd) == -1)
    {
        perror("pipe");
        return;
    }

    pid_t hub_mon_pid = fork();
    if (hub_mon_pid == -1)
    {
        perror("fork hub_mon");
        close(pipefd[0]);
        close(pipefd[1]);
        return;
    }

    if (hub_mon_pid == 0)
    {
        close(pipefd[1]); 

        pid_t mon_pid = fork();
        if (mon_pid == -1)
        {
            perror("fork monitor");
            _exit(1);
        }

        if (mon_pid == 0)
        {
            close(pipefd[0]);

            char fd_str[16];
            snprintf(fd_str, sizeof(fd_str), "%d", pipefd[1]);

            execl(MONITOR_BIN, MONITOR_BIN, fd_str, (char *)NULL);
            perror("execl monitor");
            _exit(1);
        }

        close(pipefd[1]);
        hub_mon_loop(pipefd[0]);
        close(pipefd[0]);

        waitpid(mon_pid, NULL, 0);
        _exit(0);
    }

    close(pipefd[0]);
    close(pipefd[1]);

    printf("[city_hub] start_monitor: hub_mon started (PID %d).\n", (int)hub_mon_pid);
    fflush(stdout);
}

static void cmd_calculate_scores(int district_count, char **districts)
{
    if (district_count == 0)
    {
        printf("Usage: calculate_scores <district1> [district2 ...]\n");
        return;
    }

    int (*pipes)[2] = malloc(sizeof(int[2]) * district_count);
    pid_t *pids     = malloc(sizeof(pid_t) * district_count);
    if (!pipes || !pids)
    {
        perror("malloc");
        free(pipes); free(pids);
        return;
    }

    for (int i = 0; i < district_count; i++)
    {
        if (pipe(pipes[i]) == -1)
        {
            perror("pipe scorer");
            for (int j = 0; j < i; j++) { close(pipes[j][0]); close(pipes[j][1]); }
            free(pipes); free(pids);
            return;
        }

        pid_t pid = fork();
        if (pid == -1)
        {
            perror("fork scorer");
            pids[i] = -1;
            continue;
        }

        if (pid == 0)
        {
            close(pipes[i][0]);
            if (dup2(pipes[i][1], STDOUT_FILENO) == -1)
            {
                perror("dup2");
                _exit(1);
            }
            close(pipes[i][1]);

            for (int j = 0; j < district_count; j++)
                if (j != i) { close(pipes[j][0]); close(pipes[j][1]); }

            execl(SCORER_BIN, SCORER_BIN, districts[i], (char *)NULL);
            perror("execl scorer");
            _exit(1);
        }

        close(pipes[i][1]);
        pids[i] = pid;
    }

    printf("\nWorkload Report\n");
    fflush(stdout);

    char buf[1024];
    for (int i = 0; i < district_count; i++)
    {
        if (pids[i] == -1) continue;

        printf("District: %s\n", districts[i]);
        fflush(stdout);

        ssize_t n;
        while ((n = read(pipes[i][0], buf, sizeof(buf) - 1)) > 0)
        {
            buf[n] = '\0';
            fputs(buf, stdout);
        }
        fflush(stdout);
        close(pipes[i][0]);
        waitpid(pids[i], NULL, 0);
    }

    printf("End of Workload Report\n");
    fflush(stdout);

    free(pipes);
    free(pids);
}

int main(void)
{
    printf("[city_hub] Started. Commands: start_monitor | calculate_scores <d1> [d2 ...] | exit\n");
    fflush(stdout);

    char line[512];
    while (1)
    {
        printf("city_hub> ");
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin))
            break;

        line[strcspn(line, "\n")] = '\0';

        char *tokens[64];
        int   tc = 0;
        char *tok = strtok(line, " \t");
        while (tok && tc < 63)
        {
            tokens[tc++] = tok;
            tok = strtok(NULL, " \t");
        }
        if (tc == 0) continue;

        if (strcmp(tokens[0], "exit") == 0)
        {
            printf("[city_hub] Bye.\n");
            break;
        }
        else if (strcmp(tokens[0], "start_monitor") == 0)
        {
            cmd_start_monitor();
        }
        else if (strcmp(tokens[0], "calculate_scores") == 0)
        {
            cmd_calculate_scores(tc - 1, tokens + 1);
        }
        else
        {
            printf("Unknown command: '%s'\n", tokens[0]);
        }
    }

    return 0;
}