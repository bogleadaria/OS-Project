CC     = gcc
CFLAGS = -Wall -g
 
all: city_manager monitor city_hub scorer
 
city_manager: city_manager_main.c utils.c commands.c report.h utils.h commands.h
	$(CC) $(CFLAGS) city_manager_main.c utils.c commands.c -o city_manager
 
monitor: monitor_main.c monitor_reports.c monitor_reports.h
	$(CC) $(CFLAGS) monitor_main.c monitor_reports.c -o monitor
 
city_hub: city_hub.c city_hub.h report.h
	$(CC) $(CFLAGS) city_hub.c -o city_hub
 
scorer: scorer.c report.h
	$(CC) $(CFLAGS) scorer.c -o scorer
 
clean:
	rm -f city_manager monitor city_hub scorer