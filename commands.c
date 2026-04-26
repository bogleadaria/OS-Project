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
}