#define _POSIX_C_SOURCE 200809L

#include "city_hub.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    printf("[city_hub] Started. Commands: start_monitor | calculate_scores <d1> [d2 ...] | exit\n");
    fflush(stdout);

    char line[512];
    while (1)
    {
        printf("city_hub> ");
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin))
            break;

        line[strcspn(line, "\n")] = '\0';

        char *tokens[64];
        int   tc = 0;
        char *tok = strtok(line, " \t");
        while (tok && tc < 63)
        {
            tokens[tc++] = tok;
            tok = strtok(NULL, " \t");
        }
        if (tc == 0) continue;

        if (strcmp(tokens[0], "exit") == 0)
        {
            printf("[city_hub] Bye.\n");
            break;
        }
        else if (strcmp(tokens[0], "start_monitor") == 0)
        {
            cmd_start_monitor();
        }
        else if (strcmp(tokens[0], "calculate_scores") == 0)
        {
            cmd_calculate_scores(tc - 1, tokens + 1);
        }
        else
        {
            printf("Unknown command: '%s'\n", tokens[0]);
        }
    }

    return 0;
}