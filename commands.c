#include "commands.h"
#include "report.h"
#include "utils.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

void cmd_list(const char *district)
{
    char path[256];
    snprintf(path, sizeof(path), "%s/reports.dat", district);

    struct stat st;
    if (stat(path, &st) == -1)
    {
        perror("Raports does not exist");
        return;
    }

    char permission_str[10];
    mode_to_string(st.st_mode, permission_str);
    printf("File: reports.dat | Permissions: %s | Size: %ld bytes\n", permission_str, st.st_size);

    int f = open(path, O_RDONLY);
    Report r;
    while (read(f, &r, sizeof(Report)) > 0)
    {
        printf("Report ID: %d | Category: %s | Severity: %d\n", r.report_id, r.issue_category, r.severity);
    }
    close(f);
}

void cmd_add(const char *district, const char *user)
{
    char filePath[256];
    snprintf(filePath, sizeof(filePath), "%s/reports.dat", district);

    int f = open(filePath, O_WRONLY | O_CREAT | O_APPEND, 0664);
    if (f == -1)
    {
        perror("Failed to open/create reports.dat");
        return;
    }

    chmod(filePath, 0664);
    Report newReport;
    memset(&newReport, 0, sizeof(Report));

    struct stat st;
    fstat(f, &st);
    int nextID = (st.st_size / sizeof(Report));
    newReport.report_id = nextID;
    strncpy(newReport.inspector, user, sizeof(newReport.inspector) - 1);
    newReport.latitude = 45.7489;  // TEST
    newReport.longitude = 21.2087; // TEST
    strncpy(newReport.issue_category, "road", sizeof(newReport.issue_category) - 1);
    newReport.severity = 2; // TEST
    newReport.timestamp = time(NULL);
    strncpy(newReport.description, "Pothole on Main Street", sizeof(newReport.description) - 1);

    ssize_t bytesWritten = write(f, &newReport, sizeof(Report));
    if (bytesWritten == sizeof(Report))
    {
        printf("Report (ID: %d) added successfully in %s.\n", nextID, filePath);
    }
    else
    {
        perror("Failed to write in reports.dat");
    }
    close(f);
}