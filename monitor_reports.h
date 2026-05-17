#ifndef MONITOR_REPORTS_H
#define MONITOR_REPORTS_H

#define PID_FILE ".monitor_pid"

#define MSG_INFO    "INFO:"
#define MSG_ERROR   "ERROR:"
#define MSG_EXIT    "EXIT:"

void write_pid_file(void);
void setup_signals(int pipe_fd);
int  check_existing_monitor(int pipe_fd);

#endif