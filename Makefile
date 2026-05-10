CC = gcc
CFLAGS = -Wall -g

all: city_manager monitor_reports

city_manager: city_manager_main.c utils.c commands.c
	$(CC) $(CFLAGS) city_manager_main.c utils.c commands.c -o city_manager

monitor_reports: monitor_main.c monitor_reports.c
	$(CC) $(CFLAGS) monitor_main.c monitor_reports.c -o monitor_reports

clean:
	rm -f city_manager monitor_reports