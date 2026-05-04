#ifndef MONITOR_REPORTS_H
#define MONITOR_REPORTS_H

#define PID_FILE ".monitor_pid"

void write_pid_file(void);
void setup_signals(void);

#endif