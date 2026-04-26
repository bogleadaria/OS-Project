#include "commands.h"
#include "report.h"
#include "utils.h"

#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

void cmd_list(const char* district){
    char path[256];
    snprintf(path, sizeof(path), "%s/reports.dat", district);

    struct stat st;
    if (stat(path, &st) == -1) {
        perror("Raports does not exist");
        return;
    }

    char permission_str[10];
    mode_to_string(st.st_mode, permission_str);
    printf("File: reports.dat | Permissions: %s | Size: %ld bytes\n", permission_str, st.st_size);

    int f = open(path, O_RDONLY);
    Report r;
    while(read(f, &r, sizeof(Report)) > 0) {
        printf("Report ID: %d | Category: %s | Severity: %d\n", r.report_id, r.issue_category, r.severity);
    }
    close(f);
}