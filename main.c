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

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--role") == 0 && i + 1 < argc)
        {
            strcpy(role, argv[i + 1]);
            i++;
        }
        else if (strcmp(argv[i], "--user") == 0 && i + 1 < argc)
        {
            strcpy(user, argv[i + 1]);
            i++;
        }
        else if (strcmp(argv[i], "--add") == 0 && i + 1 < argc)
        {
            strcpy(command, "add");
            strcpy(district, argv[i + 1]);
            i++;
        }
        else if (strcmp(argv[i], "--list") == 0 && i + 1 < argc)
        {
            strcpy(command, "list");
            strcpy(district, argv[i + 1]);
            i++;
        }
    }

    if (strlen(role) == 0 || strlen(user) == 0)
    {
        printf("Error: You must specify both --role and --user!\n");
        return 1;
    }

    if (strcmp(role, "manager") != 0 && strcmp(role, "inspector") != 0)
    {
        printf("Error: Invalid role! It must be 'manager' or 'inspector'.\n");
        return 1;
    }

    if (strlen(command) == 0)
    {
        printf("Error: You must specify a command (--add or --list)!\n");
        return 1;
    }

    if (strcmp(command, "add") == 0)
    {
        district_exists(district);
        log_action(district, user, role, "add");
        cmd_add(district, user);
    }
    else if (strcmp(command, "list") == 0)
    {
        log_action(district, user, role, "list");
        cmd_list(district);
    }
    else
    {
        printf("Unknown command!\n");
        return 1;
    }

    return 0;
}