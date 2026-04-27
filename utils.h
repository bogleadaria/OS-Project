#ifndef UTILS_H
#define UTILS_H

#include <sys/types.h>

void district_exists(const char* district);
void log_action(const char* district, const char* user, const char* role, const char* action);
void mode_to_string(mode_t mode, char* str);
int check_permission(const char* filePath, const char* role, int needRead, int needWrite);
void create_district_cfg(const char *district);
void create_symlink(const char *district);

#endif