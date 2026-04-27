#ifndef COMMANDS_H
#define COMMANDS_H

void cmd_add(const char *district, const char *user, const char *role);
void cmd_list(const char* district);
void cmd_view(const char *district, int report_id);
void cmd_remove_report(const char* district, const char* role, int report_id);
void cmd_update_threshold(const char* district, const char* role, int value);
void cmd_filter(const char* district, int cond_count, char **conditions);

#endif