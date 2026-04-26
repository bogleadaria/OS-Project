#ifndef REPORT_H
#define REPORT_H

#include <time.h>

#define NAME_LEN 32
#define CATEGORY_LEN 16
#define DESCRIPTION_LEN 128

typedef struct {
    int report_id;
    char inspector[NAME_LEN];
    double latitude;
    double longitude;
    char issue_category[CATEGORY_LEN];
    int severity;
    time_t timestamp;
    char description[DESCRIPTION_LEN];
} Report;

#endif