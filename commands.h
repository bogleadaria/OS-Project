#ifndef COMMANDS_H
#define COMMANDS_H

#include "report.h"

void cmd_add(const char *district, const char *user, const char *role);
void cmd_list(const char *district);
void cmd_view(const char *district, int report_id);
void cmd_remove_report(const char *district, const char *role, int report_id);
void cmd_update_threshold(const char *district, const char *role, int value);
int parse_condition(const char *input, char *field, char *op, char *value);
int match_condition(Report *r, const char *field, const char *op, const char *value);
void cmd_filter(const char *district, int cond_count, char **conditions);
void cmd_remove_district(const char *district, const char *role);

#endif