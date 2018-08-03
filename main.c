#define MAX_SIZE 1024

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <signal.h>
#include <wait.h>
#include "prompt.h"
#include "get_command.h"
#include "parse_command.h"
#include "execute_command.h"
#include "globals.h"
#include "inbuilts.h"
#include "execute_pipe.h"

int dir_length;
char home_dir[MAX_SIZE];

void signal_handler(int sig)
{
    if(sig == SIGINT)
    {
        printf("\n");
        print_prompt();
        fflush(stdout);
    }
 
    if(sig == SIGTSTP)
    {
    	int i;
    	for(i=0;i<pointer;i++)
    	{
    		printf("%d\n",i);
    		status[i] = -1;
    	}
    	if(foreground == 1)
    	{
    		jobs[pointer] = fg_id;
    		status[pointer] = -1;
    		pointer++;
    	}
        printf("\n");
        print_prompt();
        fflush(stdout);
        return;
    }

    if(sig == SIGQUIT)
    {
        printf("\n");
        print_prompt();
        fflush(stdout);
    }
 
    if(sig == SIGCHLD)
    {
    	if(foreground == 1)
    	{
    		foreground = 0;
    		return;
    	}
        union wait wstat;
        pid_t pid;

        while(1)
        {
            pid = wait3(&wstat, WNOHANG, (struct rusage *)NULL);

            if(pid == 0 || pid == -1)
                return;

            else
            {
                fprintf(stderr,"\nProcess with PID : %d exited with return value: %d\n",pid,wstat.w_retcode);
                Remove(pid);
            }
        }
    }

    return;
}

int main()
{
	char* command;
	char** parsed_commands;
	char** parsed_command;
	char cwd[MAX_SIZE];
	int i, j;

	getcwd(home_dir, MAX_SIZE);

	if(getcwd(cwd, MAX_SIZE) == NULL)
	{
		fprintf(stderr, "An error occured in getcwd() syscall\n");
		return -1;
	}

	dir_length = strlen(cwd);
	
	while(1)
	{
		signal(SIGINT, SIG_IGN);
	    signal(SIGQUIT, SIG_IGN);
	    signal(SIGTSTP, SIG_IGN);
	    signal(SIGCHLD, SIG_IGN);
	    signal(SIGTSTP, SIG_IGN);

	    if( signal(SIGINT, signal_handler) == 0 )
	        continue;
	    if( signal(SIGQUIT, signal_handler) == 0 )
	        continue;
	    if( signal(SIGTSTP, signal_handler) == 0 )
			continue;
		if( signal(SIGCHLD, signal_handler) == 0 )
			continue;
		if( signal(SIGTSTP, signal_handler) == 0 )
			continue;

		int flag, j, i;

		print_prompt();
		command = get_command();
		parsed_commands = parse_command(command, ';');
		i = 0;
		while(parsed_commands[i] != NULL)
		{
			flag = 0;
			j = 0;
			while (1)
			{
				if(parsed_commands[i][j] == '\0')
					break;
				if(parsed_commands[i][j]=='|')
				{
					flag = 1 ;
					break ;
				}
				j++ ;
			}
			if(flag == 1)
				parse_pipe(parsed_commands[i]);
			else
			{
				parsed_command = parse_command(parsed_commands[i], ' ');
				execute_command(parsed_command);
				free(parsed_command);
			}	
			i++;
		}
		free(parsed_commands);
	}
	return 0;
}
