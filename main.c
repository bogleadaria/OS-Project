#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "commands.h"

int main(int argc, char *argv[])
{
    char role[16] = "";
    char user[32] = "";
    char command[32] = "";
    char district[64] = "";
    int extra_int = -1;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--role") == 0 && i + 1 < argc)
        {
            strncpy(role, argv[i + 1], sizeof(role) - 1);
            i++;
        }
        else if (strcmp(argv[i], "--user") == 0 && i + 1 < argc)
        {
            strncpy(user, argv[i + 1], sizeof(user) - 1);
            i++;
        }
        else if (strcmp(argv[i], "--add") == 0 && i + 1 < argc)
        {
            strcpy(command, "add");
            strncpy(district, argv[i + 1], sizeof(district) - 1);
            i++;
        }
        else if (strcmp(argv[i], "--list") == 0 && i + 1 < argc)
        {
            strcpy(command, "list");
            strncpy(district, argv[i + 1], sizeof(district) - 1);
            i++;
        }
        else if (strcmp(argv[i], "--view") == 0 && i + 2 < argc)
        {
            strcpy(command, "view");
            strncpy(district, argv[i + 1], sizeof(district) - 1);
            extra_int = atoi(argv[i + 2]);
            i += 2;
        }
    }

    if (strlen(role) == 0 || strlen(user) == 0)
    {
        printf("Error: You must specify both --role and --user!\n");
        return 1;
    }

    if (strcmp(role, "manager") != 0 && strcmp(role, "inspector") != 0)
    {
        printf("Error: Invalid role '%s'. Must be 'manager' or 'inspector'.\n", role);
        return 1;
    }

    if (strlen(command) == 0)
    {
        printf("Error: You must specify a command!\n");
        printf("Available commands: --add, --list, --view.\n");
        return 1;
    }
    if (strcmp(command, "add") == 0)
    {
        district_exists(district);
        create_district_cfg(district);
        log_action(district, user, role, "add");
        cmd_add(district, user, role);
    }
    else if (strcmp(command, "list") == 0)
    {
        log_action(district, user, role, "list");
        cmd_list(district);
    }
    else if (strcmp(command, "view") == 0)
    {
        if (extra_int == -1)
        {
            printf("Error: --view requires <district> <report_id>\n");
            return 1;
        }
        log_action(district, user, role, "view");
        cmd_view(district, extra_int);
    }
    else
    {
        printf("Unknown command: %s\n", command);
        return 1;
    }

    return 0;
}