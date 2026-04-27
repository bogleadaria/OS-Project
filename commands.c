#include "commands.h"
#include "report.h"
#include "utils.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
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
    if(f == -1){
        perror("Failed to open district.cfg.\n");
        return;
    }

    char content[64];
    int len = snprintf(content, sizeof(content), "severity_threshold = %d\n", value);
    write(f,content, len);
    close(f);

    printf("Severity threshold updated to %d in %s.\n", value, district);
}

void cmd_filter(const char *district, int cond_count, char **conditions)
{
}