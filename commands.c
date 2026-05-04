#include "commands.h"
#include "report.h"
#include "utils.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

void cmd_list(const char *district)
{
    char path[256];
    snprintf(path, sizeof(path), "%s/reports.dat", district);

    struct stat st;
    if (stat(path, &st) == -1)
    {
        perror("reports.dat does not exist");
        return;
    }

    char permission_str[10];
    mode_to_string(st.st_mode, permission_str);

    char time_str[64];
    struct tm *tm_info = localtime(&st.st_mtime);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);

    printf("File: reports.dat | Permissions: %s | Size: %ld bytes | Last modified: %s\n",
           permission_str, (long)st.st_size, time_str);

    int f = open(path, O_RDONLY);
    if (f == -1)
    {
        perror("Failed to open reports.dat");
        return;
    }

    Report r;
    int count = 0;
    while (read(f, &r, sizeof(Report)) == sizeof(Report))
    {
        char report_time[64];
        struct tm *rt = localtime(&r.timestamp);
        strftime(report_time, sizeof(report_time), "%Y-%m-%d %H:%M:%S", rt);

        printf("ID: %d | Inspector: %s | Category: %s | Severity: %d | Time: %s\n",
               r.report_id, r.inspector, r.issue_category, r.severity, report_time);
        count++;
    }
    close(f);

    if (count == 0)
    {
        printf("No reports found in %s.\n", district);
    }
}

void cmd_add(const char *district, const char *user, const char *role)
{
    char filePath[256];
    snprintf(filePath, sizeof(filePath), "%s/reports.dat", district);

    struct stat st;
    if (stat(filePath, &st) == 0)
    {
        if (!check_permission(filePath, role, 0, 1))
        {
            return;
        }
    }
    int f = open(filePath, O_WRONLY | O_CREAT | O_APPEND, 0664);
    if (f == -1)
    {
        perror("Failed to open/create reports.dat");
        return;
    }

    chmod(filePath, 0664);

    Report newReport;
    memset(&newReport, 0, sizeof(Report));

    fstat(f, &st);
    int nextID = (int)(st.st_size / sizeof(Report));
    newReport.report_id = nextID;

    strncpy(newReport.inspector, user, sizeof(newReport.inspector) - 1);

    printf("Latitude: ");
    scanf("%lf", &newReport.latitude);

    printf("Longitude: ");
    scanf("%lf", &newReport.longitude);

    printf("Issue category (road/ light/ flooding/ etc): ");
    scanf("%15s", newReport.issue_category);

    printf("Severity (1=minor, 2=moderate, 3=critical): ");
    scanf("%d", &newReport.severity);

    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;

    printf("Description: ");
    fgets(newReport.description, sizeof(newReport.description), stdin);

    newReport.description[strcspn(newReport.description, "\n")] = '\0';

    newReport.timestamp = time(NULL);

    ssize_t bytesWritten = write(f, &newReport, sizeof(Report));
    if (bytesWritten == sizeof(Report))
    {
        printf("Report (ID: %d) added successfully in %s.\n", nextID, filePath);
    }
    else
    {
        perror("Failed to write report");
    }

    close(f);

    create_symlink(district);
}

void cmd_view(const char *district, int report_id)
{
    char path[256];
    snprintf(path, sizeof(path), "%s/reports.dat", district);

    int f = open(path, O_RDONLY);
    if (f == -1)
    {
        perror("Failed to open reports.dat");
        return;
    }

    Report r;
    int found = 0;
    while (read(f, &r, sizeof(Report)) == sizeof(Report))
    {
        if (r.report_id == report_id)
        {
            found = 1;
            char time_str[64];
            struct tm *tm_info = localtime(&r.timestamp);
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);

            printf("Report ID: %d\n", r.report_id);
            printf("Inspector: %s\n", r.inspector);
            printf("Latitude: %.6f\n", r.latitude);
            printf("Longitude: %.6f\n", r.longitude);
            printf("Category: %s\n", r.issue_category);
            printf("Severity: %d\n", r.severity);
            printf("Timestamp: %s\n", time_str);
            printf("Description: %s\n", r.description);
            break;
        }
    }
    close(f);

    if (!found)
    {
        printf("Report with ID %d not found in %s.\n", report_id, district);
    }
}

void cmd_remove_report(const char *district, const char *role, int report_id)
{
    if (strcmp(role, "manager") != 0)
    {
        printf("Permission denied: only managers can remove reports.\n");
        return;
    }
    char path[256];
    snprintf(path, sizeof(path), "%s/reports.dat", district);

    if (!check_permission(path, role, 1, 1))
    {
        return;
    }

    int f = open(path, O_RDWR);
    if (f == -1)
    {
        perror("Failed to open reports.dat");
        return;
    }

    struct stat st;
    fstat(f, &st);
    int total_reports = (int)(st.st_size / sizeof(Report));
    int found_pos = -1;

    Report r;
    for (int i = 0; i < total_reports; i++)
    {
        lseek(f, (off_t)(i * sizeof(Report)), SEEK_SET);
        read(f, &r, sizeof(Report));
        if (r.report_id == report_id)
        {
            found_pos = i;
            break;
        }
    }
    if (found_pos == -1)
    {
        printf("Report with ID %d not found.\n", report_id);
        close(f);
        return;
    }

    for (int i = found_pos + 1; i < total_reports; i++)
    {
        lseek(f, (off_t)(i * sizeof(Report)), SEEK_SET);
        read(f, &r, sizeof(Report));

        lseek(f, (off_t)((i - 1) * sizeof(Report)), SEEK_SET);
        write(f, &r, sizeof(Report));
    }
    ftruncate(f, (off_t)((total_reports - 1) * sizeof(Report)));

    close(f);

    printf("Report ID %d removed successfully from %s.\n", report_id, district);
}

void cmd_update_threshold(const char *district, const char *role, int value)
{
    if (strcmp(role, "manager") != 0)
    {
        printf("Permission denied: only managers can update the threshold.\n");
        return;
    }

    char filePath[256];
    snprintf(filePath, sizeof(filePath), "%s/district.cfg", district);

    struct stat st;
    if (stat(filePath, &st) == -1)
    {
        printf("district.cfg not found. Creating it ...\n");
        create_district_cfg(district);
        stat(filePath, &st);
    }

    mode_t expected = S_IRUSR | S_IWUSR | S_IRGRP;
    mode_t actual = st.st_mode & 0777;

    if (actual != expected)
    {
        char perm_str[10];
        mode_to_string(st.st_mode, perm_str);
        printf("Security error: district.cfg has unexpected permissions %s (expected rw-r-----).\nRefusing to write.\n", perm_str);
        return;
    }

    int f = open(filePath, O_WRONLY | O_TRUNC);
    if (f == -1)
    {
        perror("Failed to open district.cfg.\n");
        return;
    }

    char content[64];
    int len = snprintf(content, sizeof(content), "severity_threshold = %d\n", value);
    write(f, content, len);
    close(f);

    printf("Severity threshold updated to %d in %s.\n", value, district);
}

int parse_condition(const char *input, char *field, char *op, char *value)
{
    char buf[256]; // string copy
    strncpy(buf, input, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    // field
    char *first_colon = strchr(buf, ':');
    if (!first_colon)
        return 0;
    *first_colon = '\0';

    // operator
    char *second_colon = strchr(first_colon + 1, ':');
    if (!second_colon)
        return 0;
    *second_colon = '\0';

    strncpy(field, buf, 32 - 1);
    field[32 - 1] = '\0';

    strncpy(op, first_colon + 1, 4 - 1);
    op[4 - 1] = '\0';

    strncpy(value, second_colon + 1, 64 - 1);
    value[64 - 1] = '\0';

    return 1;
}

int match_condition(Report *r, const char *field, const char *op, const char *value)
{
    if (strcmp(field, "severity") == 0)
    {
        int sev_val = atoi(value);
        if (strcmp(op, "==") == 0)
            return r->severity == sev_val;
        if (strcmp(op, "!=") == 0)
            return r->severity != sev_val;
        if (strcmp(op, "<") == 0)
            return r->severity < sev_val;
        if (strcmp(op, "<=") == 0)
            return r->severity <= sev_val;
        if (strcmp(op, ">") == 0)
            return r->severity > sev_val;
        if (strcmp(op, ">=") == 0)
            return r->severity >= sev_val;
    }
    else if (strcmp(field, "category") == 0)
    {
        int cmp = strcmp(r->issue_category, value);
        if (strcmp(op, "==") == 0)
            return cmp == 0;
        if (strcmp(op, "!=") == 0)
            return cmp != 0;
    }
    else if (strcmp(field, "inspector") == 0)
    {
        int cmp = strcmp(r->inspector, value);
        if (strcmp(op, "==") == 0)
            return cmp == 0;
        if (strcmp(op, "!=") == 0)
            return cmp != 0;
    }
    else if (strcmp(field, "timestamp") == 0)
    {
        time_t ts_val = (time_t)atol(value);
        if (strcmp(op, "==") == 0)
            return r->timestamp == ts_val;
        if (strcmp(op, "!=") == 0)
            return r->timestamp != ts_val;
        if (strcmp(op, "<") == 0)
            return r->timestamp < ts_val;
        if (strcmp(op, "<=") == 0)
            return r->timestamp <= ts_val;
        if (strcmp(op, ">") == 0)
            return r->timestamp > ts_val;
        if (strcmp(op, ">=") == 0)
            return r->timestamp >= ts_val;
    }

    return 0;
}

void cmd_filter(const char *district, int cond_count, char **conditions)
{
    char path[256];
    snprintf(path, sizeof(path), "%s/reports.dat", district);

    int f = open(path, O_RDONLY);
    if (f == -1)
    {
        perror("Failed to open reports.dat");
        return;
    }

    char fields[10][32];
    char ops[10][4];
    char values[10][64];

    for (int i = 0; i < cond_count; i++)
    {
        if (!parse_condition(conditions[i], fields[i], ops[i], values[i]))
        {
            printf("Invalid condition format: '%s'. Expected field:op:value\n", conditions[i]);
            close(f);
            return;
        }
    }

    Report r;
    int found = 0;
    while (read(f, &r, sizeof(Report)) == sizeof(Report))
    {
        int all_match = 1;
        for (int i = 0; i < cond_count; i++)
        {
            if (!match_condition(&r, fields[i], ops[i], values[i]))
            {
                all_match = 0;
                break;
            }
        }

        if (all_match)
        {
            char time_str[64];
            struct tm *tm_info = localtime(&r.timestamp);
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);

            printf("ID: %d | Inspector: %s | Category: %s | Severity: %d | Time: %s | Desc: %s\n",
                   r.report_id, r.inspector, r.issue_category, r.severity, time_str, r.description);
            found++;
        }
    }
    close(f);

    if (found == 0)
    {
        printf("No reports match the given conditions.\n");
    }
    else
    {
        printf("Total matching reports: %d\n", found);
    }
}

void cmd_remove_district(const char *district, const char *role)
{
    if (strcmp(role, "manager") != 0)
    {
        printf("Permission denied: only managers can remove a district!\n");
        return;
    }

    if (strchr(district, '/') != NULL || strstr(district, "...") != NULL)
    {
        printf("Error: invalid district name '%s'!\n", district);
        return;
    }

    struct stat st;
    if (stat(district, &st) == -1)
    {
        printf("Error: district '%s' doesn't exist!\n", district);
        return;
    }

    pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork failed!");
        return;
    }
    if (pid == 0)
    {
        execlp("rm", "rm", "-rf", district, (char *)NULL);
        perror("execlp rm failed!");
        _exit(1);
    }

    int status;
    waitpid(pid, &status, 0);
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
    {
        printf("Error: failed to remove district directory '%s'.\n", district);
        return;
    }

    char link_name[256];
    snprintf(link_name, sizeof(link_name), "active_reports-%s", district);

    struct stat lst;
    if (lstat(link_name, &lst) == 0)
    {
        if (unlink(link_name) == -1)
        {
            perror("Warning: couldn't remove symlink");
        }
        else
        {
            printf("Symlink '%s' removed.\n", district);
        }
    }
    printf("District '%s' has been removed successfully.\n", district);
}