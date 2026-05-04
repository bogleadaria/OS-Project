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
    int extra_int = -1;  // used for report_id or threshold value
    int cond_start = -1; // index of first condition --filter
    int cond_count = 0;

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
        else if (strcmp(argv[i], "--remove_report") == 0 && i + 2 < argc)
        {
            strcpy(command, "remove_report");
            strncpy(district, argv[i + 1], sizeof(district) - 1);
            extra_int = atoi(argv[i + 2]);
            i += 2;
        }
        else if (strcmp(argv[i], "--update_threshold") == 0 && i + 2 < argc)
        {
            strcpy(command, "update_threshold");
            strncpy(district, argv[i + 1], sizeof(district) - 1);
            extra_int = atoi(argv[i + 2]);
            i += 2;
        }
        else if (strcmp(argv[i], "--filter") == 0 && i + 1 < argc)
        {
            strcpy(command, "filter");
            strncpy(district, argv[i + 1], sizeof(district) - 1);
            i++;
            cond_start = i + 1;
            cond_count = argc - cond_start;
            i = argc;
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
        printf("Available commands: --add, --list, --view, --remove_report, --update_threshold.\n");
        return 1;
    }

    check_dangling_symlinks();

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
    else if (strcmp(command, "remove_report") == 0)
    {
        if (extra_int == -1)
        {
            printf("Error: --remove_report requires <district> <report_id>\n");
            return 1;
        }
        log_action(district, user, role, "remove_report");
        cmd_remove_report(district, role, extra_int);
    }
    else if (strcmp(command, "update_threshold") == 0)
    {
        if (extra_int == -1)
        {
            printf("Error: --update_threshold requires <district> <value>\n");
            return 1;
        }
        log_action(district, user, role, "update_threshold");
        cmd_update_threshold(district, role, extra_int);
    }
    else if (strcmp(command, "filter") == 0)
    {
        if (cond_count <= 0)
        {
            printf("Error: --filter requires at least one condition (field:operator:value)\n");
            return 1;
        }
        log_action(district, user, role, "filter");
        cmd_filter(district, cond_count, &argv[cond_start]);
    }
    else
    {
        printf("Unknown command: %s\n", command);
        return 1;
    }

    return 0;
}