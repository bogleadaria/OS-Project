CC = gcc
CFLAGS = -Wall -g
Target = city_manager
SRCS = city_manager_main.c utils.c commands.c
$(Target): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(Target)
clean:
	rm -f $(Target)