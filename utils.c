#include "utils.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

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

    snprintf(link_name, sizeof(link_name), "active-reports-%s", district);
    snprintf(target, sizeof(target), "%s/reports.dat", district);

    struct stat lst;
    if(lstat(link_name, &lst) == 0){
        unlink(link_name);
    }
    if(symlink(target, link_name) == -1){
        perror("Failed to create symlink.");
    }
}