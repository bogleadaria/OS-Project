#ifndef UTILS_H
#define UTILS_H

#include <sys/types.h>

void district_exists(const char* district);
void log_action(const char* district, const char* user, const char* role, const char* action);
void mode_to_string(mode_t mode, char* str);

#endif