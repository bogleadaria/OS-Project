#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

void district_exists(const char *district)
{
    struct stat st;
    if (stat(district, &st) == -1)
    {
        mkdir(district, 0750);
        chmod(district, 0750);
    }
}

void log_action(const char *district, const char *user, const char *role, const char *action)
{
    if (strcmp(role, "inspector") == 0)
    {
        char filePath[256];
        snprintf(filePath, sizeof(filePath), "%s/logged_district", district);
        struct stat st;

        if (stat(filePath, &st) == 0)
        {
            if (!(st.st_mode & S_IWGRP))
            {
                printf("Permission denied: Inspector cannot write to logged_district.\n");
                return;
            }
        }
    }

    char filePath[256];
    snprintf(filePath, sizeof(filePath), "%s/logged_district", district);

    int f = open(filePath, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (f != -1)
    {
        chmod(filePath, 0644);

        time_t now = time(NULL);
        char entry[512];
        int len = snprintf(entry, sizeof(entry), "%ld\n%s\n%s %s\n", now, user, role, action);
        write(f, entry, len);

        close(f);
    }
}

void mode_to_string(mode_t mode, char *str)
{
    str[0] = (mode & S_IRUSR) ? 'r' : '-';
    str[1] = (mode & S_IWUSR) ? 'w' : '-';
    str[2] = (mode & S_IXUSR) ? 'x' : '-';
    str[3] = (mode & S_IRGRP) ? 'r' : '-';
    str[4] = (mode & S_IWGRP) ? 'w' : '-';
    str[5] = (mode & S_IXGRP) ? 'x' : '-';
    str[6] = (mode & S_IROTH) ? 'r' : '-';
    str[7] = (mode & S_IWOTH) ? 'w' : '-';
    str[8] = (mode & S_IXOTH) ? 'x' : '-';
    str[9] = '\0';
}

int check_permission(const char *filePath, const char *role, int needRead, int needWrite)
{
    struct stat st;
    if (stat(filePath, &st) == -1)
    {
        perror("Stat failed");
        return 0;
    }

    mode_t mode = st.st_mode;

    if (strcmp(role, "manager") == 0)
    {
        if (needRead && !(mode & S_IRUSR))
        {
            printf("Permission denied: manager cannot read '%s'.\n", filePath);
            return 0;
        }
        if (needWrite && !(mode & S_IWUSR))
        {
            printf("Permission denied: manager cannot write to '%s'.\n", filePath);
            return 0;
        }
    }
    else if (strcmp(role, "inspector") == 0)
    {
        if (needRead && !(mode & S_IRGRP))
        {
            printf("Permission denied: inspector cannot read '%s'.\n", filePath);
            return 0;
        }
        if (needWrite && !(mode & S_IWGRP))
        {
            printf("Permission denied: inspector cannot write to '%s'.\n", filePath);
            return 0;
        }
    }
    return 1;
}

void create_district_cfg(const char *district)
{
    char filePath[256];
    snprintf(filePath, sizeof(filePath), "%s/district.cfg", district);

    struct stat st;
    if (stat(filePath, &st) == -1)
    {
        int f = open(filePath, O_WRONLY | O_CREAT | O_TRUNC, 0640);
        if (f == -1)
        {
            perror("Failed to create district.cfg");
            return;
        }

        const char *default_content = "severity_treshhold = 1\n";
        write(f, default_content, strlen(default_content));

        close(f);

        chmod(filePath, 0640);
    }
}

void create_symlink(const char *district)
{
    char link_name[256];
    char target[256];

    snprintf(link_name, sizeof(link_name), "active_reports-%s", district);
    snprintf(target, sizeof(target), "%s/reports.dat", district);

    struct stat lst;
    if (lstat(link_name, &lst) == 0)
    {
        unlink(link_name);
    }
    if (symlink(target, link_name) == -1)
    {
        perror("Failed to create symlink.");
    }
}

void check_dangling_symlinks()
{
    char link_name[256];
    char target[256];
    struct stat lst, st;

    DIR *dir = opendir(".");
    if (dir == NULL)
    {
        perror("Failed to open current directory.\n");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strncmp(entry->d_name, "active_reports-", 15) != 0)

            continue;

        strncpy(link_name, entry->d_name, sizeof(link_name) - 1);

        if (lstat(link_name, &lst) == -1)
            continue;

        if (!S_ISLNK(lst.st_mode))
            continue;

        ssize_t len = readlink(link_name, target, sizeof(target) - 1);
        if (len == -1)
            continue;

        target[len] = '\0';

        if(stat(target, &st) == -1){
            printf("Warning: dangling symlink detected: %s - %s (target does not exist).\n", link_name, target);
        }
    }
    closedir(dir);
}

void notify_monitor(const char *district, const char *user){
    char log_path[256];
    snprintf(log_path, sizeof(log_path), "%s/logged_district", district);

    int f = open(".monitor_pid", O_RDONLY);
    if (f == -1){
        int log = open(log_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
        if(log != -1 ){
            const char *msg = "MONITOR NOTIFICATION: monitor couldn't be informed (no .monitor_pid file).\n";
            write(log, msg, strlen(msg));
            close(log);
        }
        return;
    }
    char pid_str[32];
    memset(pid_str, 0, sizeof(pid_str));
    read(f, pid_str, sizeof(pid_str) -1);
    close(f);

    pid_t monitor_pid = (pid_t)atoi(pid_str);
    if(monitor_pid <= 0){
        int log = open(log_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
        if(log != -1){
            const char *msg = "MONITOR NOTIFICATION: monitor couldn't be informed (invalid pid).\n";
            write(log, msg, strlen(msg));
            close(log);
        }
        return;
    }
    if(kill(monitor_pid, SIGUSR1) == -1){
        int log = open(log_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
        if(log != -1){
            const char *msg = "MONITOR NOTIFICATION: monitor couldn't be informed (kill failed).\n";
            write(log, msg, strlen(msg));
            close(log);
        }
        return;
    }
    int log = open(log_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if(log != -1){
        const char *msg= "MONITOR NOTIFICATION: monitor successfully notified via SIGUSR1.\n";
        write(log, msg, strlen(msg));
        close(log);
    }
}