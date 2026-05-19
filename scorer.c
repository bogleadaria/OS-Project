#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "report.h"

#define MAX_INSPECTORS 64

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: scorer <district>\n");
        return 1;
    }

    const char *district = argv[1];
    const path[256];
    snprintf(path, sizeof(path), "%s/reports.dat", district);

    int f = open(path, O_RDONLY);
    if (f == -1)
    {
        fprintf(stderr, "scorer: cannot open %s\n");
        return 1;
    }

    char inspectors[MAX_INSPECTORS][32];
    int scores[MAX_INSPECTORS];
    int count = 0;

    Report r;
    while (read(f, &r, sizeof(Report)) == sizeof(Report))
    {
        int found = -1;
        for (int i = 0; i < count; i++)
        {
            if (strcmp(inspectors[i], r.inspector) == 0)
            {
                found = i;
                break;
            }
        }

        if (found == -1)
        {
            strncpy(inspectors[count], r.inspector, 31);
            inspectors[count][31] = '\0';
            scores[count] = r.severity;
            count++;
        }
        else
        {
            scores[found] += r.severity;
        }
    }
    close(f);

    for(int i = 0; i < count; i++){
        printf("Inspector: %-20s | Workload score: %d\n", inspectors[i], scores[i]);
    }

    if(count == 0){
        printf("No reports found.\n");
    }

    return 0;
}