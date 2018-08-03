#define MAX_SIZE 4100

#include <stdio.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <string.h>
#include "globals.h"

void substr(char cwd[], int l, int r)
{
	int i, len;
	for(i=l;i<=r;i++)
		cwd[i-l] = cwd[i];
	cwd[r-l+1] = '\0';
	return;
}

void print_prompt()
{
	int check_uname, check_gethostname, cwd_length;
	char host_name[MAX_SIZE], cwd[MAX_SIZE];
	struct utsname buf;
	
	check_uname = uname(&buf);

	if(check_uname != 0)
	{
		fprintf(stderr, "An error occured in uname() syscall\n");
		return;
	}

	check_gethostname =  gethostname(host_name, MAX_SIZE);

	if(check_gethostname != 0)
	{
		fprintf(stderr, "An error occured in gethostname() syscall\n");
		return;
	}

	if(getcwd(cwd, MAX_SIZE) == NULL)
	{
		fprintf(stderr, "An error occured in getcwd() syscall\n");
		return;
	}

	cwd_length = strlen(cwd);

	substr(cwd, dir_length, cwd_length-1);

	printf("<%s@%s:~%s> ", host_name, buf.sysname, cwd);
	return;
}
