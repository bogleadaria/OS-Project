#ifndef CITY_HUB_H
#define CITY_HUB_H

static void hub_mon_loop(int read_fd);
void cmd_start_monitor(void);
void cmd_calculate_scores(int district_count, char **districts);

#endif