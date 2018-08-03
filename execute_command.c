#define MAX_SIZE 4100

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <wait.h>
#include "globals.h"
#include "inbuilts.h"

int check_parameters(char** command)
{
	int i = 0;
	while(command[i] != NULL)
	{
		if(!strcmp(command[i], "&") && command[i+1]==NULL)
			return 1;
		i++;
	}
	return 0;
}

void execute_command(char** command)
{
	if(!strcmp(command[0], "quit"))
		execute_quit(command);

	else if(check_parameters(command))
		execute_background(command);

	else if(!strcmp(command[0], "pwd"))
		execute_pwd(command);

	else if(!strcmp(command[0], "cd"))
		execute_cd(command);

	// else if(!strcmp(command[0], "echo"))
	// 	execute_echo(command);

	else if(!strcmp(command[0], "ls"))
		execute_ls(command);

	else if(!strcmp(command[0], "pinfo"))
		execute_pinfo(command);

	else if(!strcmp(command[0], "nightswatch"))
		execute_nightswatch(command);

	else if(!strcmp(command[0], "setenv"))
		execute_setenv(command);

	else if(!strcmp(command[0], "unsetenv"))
		execute_unsetenv(command);

	else if(!strcmp(command[0], "jobs"))
		execute_jobs(command);

	else if(!strcmp(command[0], "kjob"))
		execute_kjob(command);

	else if(!strcmp(command[0], "overkill"))
		execute_overkill(command);

	else if(!strcmp(command[0], "fg"))
		execute_fg(command);

	else if(!strcmp(command[0], "bg"))
		execute_bg(command);

	else
		execute_foreground(command);

	return;
}
