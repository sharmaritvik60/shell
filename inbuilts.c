#define MAX_SIZE 4100

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <wait.h>
#include <signal.h>
#include <termios.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <pwd.h>
#include <grp.h>
#include "globals.h"
#include "prompt.h"
#include "redirect.h"

int jobs[1024];
int status[1024];
int pointer = 0;
int foreground = 0;
int fg_id;

void Remove(int pid)
{
	int i, flag=0;

	for(i=0;i<pointer;i++)
	{
		if(jobs[i] == pid)
		{
			flag = 1;
			break;
		}
	}

	if(flag)
	{
		for(;i<pointer-1;i++)
		{
			jobs[i] = jobs[i+1];
			status[i] = status[i+1];
		}

		pointer--;
	}

	return;
}

int check_integer(char* string)
{
	int i = 0;
	while(*(string+i) != '\0')
	{
		if(*(string+i)<'0' || *(string+i)>'9')
			return 0;
		i++;
	}
	return 1;
}

int string_to_integer(char* str)
{
    int mult = 1;
    int re=0, i;
    int len = strlen(str);
    for(i = len -1 ; i >= 0 ; i--)
    {
        re = re + ((int)str[i] -48) * mult;
        mult = mult*10;
    }

    return re;
}

void print_file_details(struct stat file_stat, int flag)
{
	char *time;

	if(flag==0 || flag==1)
		return;

	printf( (S_ISDIR(file_stat.st_mode)) ? "d" : "-");
	printf( (file_stat.st_mode & S_IRUSR) ? "r" : "-");
	printf( (file_stat.st_mode & S_IWUSR) ? "w" : "-");
	printf( (file_stat.st_mode & S_IXUSR) ? "x" : "-");
	printf( (file_stat.st_mode & S_IRGRP) ? "r" : "-");
	printf( (file_stat.st_mode & S_IWGRP) ? "w" : "-");
	printf( (file_stat.st_mode & S_IXGRP) ? "x" : "-");
	printf( (file_stat.st_mode & S_IROTH) ? "r" : "-");
	printf( (file_stat.st_mode & S_IWOTH) ? "w" : "-");
	printf( (file_stat.st_mode & S_IXOTH) ? "x " : "- ");

	printf("%ld     ",file_stat.st_nlink);

	struct passwd *pw = getpwuid(file_stat.st_uid);
	struct group  *gr = getgrgid(file_stat.st_gid);

	printf("%s   %s   ",pw->pw_name, gr->gr_name);

	printf("%ld       ", file_stat.st_size);

	struct tm* timeinfo = localtime(&file_stat.st_mtime);
	time = asctime(timeinfo);
	time[strlen(time)-1] = '\0';
	printf("%s       ", time);

	return;
}

int modify(char *field)
{
	int i = 0, count = 0;
	while(field[i] != '\0')
	{
		if(field[i] == ' ')
		{
			i++;
			continue;
		}
		if(field[i]<'0' || field[i]>'9')
		{
			field[i] = '\0';
			return count;
		}
		if(field[i]>='0' && field[i]<='9' && field[i-1]==' ')
			count++;
		i++;
	}
	return count;
}

void print_cpu(int count)
{
	int i;
	printf("       CPU0");
	for(i=1;i<count;i++)
		printf("       CPU%d",i);
	printf("\n\n");
	return;
}

void execute_quit(char** command)
{
	if(command[1] == NULL)
		exit(0);
	else
		fprintf(stderr, "Invalid syntax\n");
	return;
}

void execute_pwd(char** command)
{
	if(command[1] == NULL)
	{
		char cwd[MAX_SIZE];

		if(getcwd(cwd, MAX_SIZE) == NULL)
		{
			fprintf(stderr, "Error occured in accessing current directory via getcwd()\n");
			return;
		}

		int cwd_length = strlen(cwd);
		substr(cwd, dir_length, cwd_length-1);
		printf("~%s\n",cwd);
		return;
	}
	else
	{
		fprintf(stderr, "Incorrect syntax for command\n");
		return;
	}
}

void execute_cd(char** command)
{
	if(command[1]==NULL || !strcmp(command[1],"~"))
	{
		if((chdir(home_dir)) < 0)
			fprintf(stderr, "Error while changing directory\n");
		return;
	}

	if(command[1] == NULL || command[2] != NULL)
	{
		fprintf(stderr, "Incorrect syntax for command\n");
		return;
	}
	else
	{
		if(chdir(command[1]) < 0)
			fprintf(stderr, "Error while changing directory\n");
		return;
	}
}

void execute_echo(char** command)
{
	int i = 1;
	while(command[i] != NULL)
	{
		printf("%s ",command[i]);
		i++;
	}
	printf("\n");
	return;
}

void execute_ls(char** command)
{
	int flag1, flag2, dir_no;
	flag2 = 0;
	if(command[1] == NULL)
		flag1 = 0;
	else if(strcmp(command[1], "-l") && strcmp(command[1], "-a") && strcmp(command[1], "-la") && strcmp(command[1], "-al") && command[2]==NULL)
	{
		flag1 = 0;
		flag2 = 1;
		dir_no = 1;
	}		
	else if(!strcmp(command[1], "-l") && command[2]==NULL)
		flag1 = 2;
	else if(!strcmp(command[1], "-a") && command[2]==NULL)
		flag1 = 1;
	else if((!strcmp(command[1], "-la") || !strcmp(command[1], "-al")) && command[2]==NULL)
		flag1 = 3;
	else if(!strcmp(command[1], "-l") && strcmp(command[2], "-a") && strcmp(command[2], "-la") && strcmp(command[2], "-al") && command[3]==NULL)
	{
		flag1 = 2;
		flag2 = 1;
		dir_no = 2;
	}
	else if(!strcmp(command[1], "-a") && strcmp(command[2], "-l") && strcmp(command[2], "-la") && strcmp(command[2], "-al") && command[3]==NULL)
	{
		flag1 = 1;
		flag2 = 1;
		dir_no = 2;
	}
	else if(!strcmp(command[1], "-la") && strcmp(command[2], "-a") && strcmp(command[2], "-l") && strcmp(command[2], "-al") && command[3]==NULL)
	{
		flag1 = 3;
		flag2 = 1;
		dir_no = 2;
	}
	else if(!strcmp(command[1], "-al") && strcmp(command[2], "-a") && strcmp(command[2], "-la") && strcmp(command[2], "-l") && command[3]==NULL)
	{
		flag1 = 3;
		flag2 = 1;
		dir_no = 2;
	}
	else if((!strcmp(command[1], "-l") && !strcmp(command[2], "-a")) || (!strcmp(command[1], "-a") && !strcmp(command[2], "-l")) && (command[3]==NULL))
		flag1 = 3;
	else if(((!strcmp(command[1], "-l") && !strcmp(command[2], "-a")) || (!strcmp(command[1], "-a") && !strcmp(command[2], "-l"))) && (strcmp(command[3], "-la") && strcmp(command[3], "-al") && command[4]==NULL))
	{
		flag1 = 3;
		flag2 = 1;
		dir_no = 3;
	}
	else
	{
		fprintf(stderr, "Incorrect syntax\n");
		return;
	}

	DIR *dp;
	struct dirent* dirp;
	char cwd[MAX_SIZE];

	if(flag2 == 0)
	{
		if(getcwd(cwd, MAX_SIZE) == NULL)
		{
			fprintf(stderr, "An error occured in getcwd() syscall\n");
			return;
		}

		if((dp=opendir(cwd)) == NULL)
		{
			fprintf(stderr, "Can't open directory %s\n",cwd);
			return;
		}
	}

	else
	{
		if((dp=opendir(command[dir_no])) == NULL)
		{
			fprintf(stderr, "Can't find directory %s\n",command[dir_no]);
			return;
		}
	}

	while((dirp = readdir(dp)) != NULL)
	{
		struct stat file_stat;
		if(dirp->d_name[0]=='.' && (flag1==0 || flag1==2))
			continue;
		stat(dirp->d_name, &file_stat);			
		print_file_details(file_stat, flag1);
		printf("%s\n",dirp->d_name);
	}

	return;
}

void execute_background(char** command)
{
	char *argv[2];
	int i = 0;
	
	while(strcmp(command[i], "&"))
		i++;

	argv[0] = command[0];
	command[i] = NULL;
	
	int pid = fork();

	if(pid == 0)
	{			
		if(execvp(command[0], command) == -1)
		{
			fprintf(stderr, "Command doesn't exist\n");
			return;
		}
	}

	else if(pid < 0)
	{
		fprintf(stderr, "Failed\n");
		return;
	}
	
	else
	{
		fprintf(stderr, "Process with pid %d has started in background\n",pid);
		
		jobs[pointer] = pid;
		status[pointer] = 1;
		pointer++;
	}
	
	return;
}

void execute_foreground(char** command)
{
	char *argv[2];
	argv[0] = command[0];
	argv[1] = NULL;
	int pid = fork();

	if(pid==0)
	{
		redirect(command);

		if(execvp(command[0], command) == -1)
		{
			fprintf(stderr, "Command not found\n");
			return;
		}
	}

	else if (pid < 0)
	{
		fprintf(stderr, "Failed\n");
		return;
	}

	else
	{
		foreground = 1;
		fg_id = pid;
		siginfo_t* infop;
		waitid(P_PID, pid, infop, WUNTRACED);
	}

	return;
}

void execute_pinfo(char** command)
{
	int flag = 0, ret;
	char file[MAX_SIZE], s[MAX_SIZE], c, l[MAX_SIZE], xyz[MAX_SIZE], path[MAX_SIZE], exe[MAX_SIZE];
	strcpy(file, "/proc/");
	if(command[1] == NULL)
	{
		int pid = getpid(), j;
		int index = 0;
		int n = pid;
		do
		{
			s[index] = n%10 + '0';
			index++;
			n = n/10;
		}
		while(n>0);
		s[index] = '\0';
		j = index - 1;
		index = 0;
		while(index<j)
		{
			c = s[index];
			s[index] = s[j];
			s[j] = c;
			index++; j--;
		}
		strcat(file, s);
	}
	else
	{
		strcat(file, command[1]);
		flag = 1;
	}

	strcpy(xyz, file);
	strcpy(path, file);
	strcat(xyz, "/status");

	FILE* fp = fopen(xyz, "r");
	
	if(!fp)
	{
		fprintf(stderr, "Invalid process\n");
		return;
	}

	while(fgets(l, sizeof l, fp))
	{
		char* field = strtok(l, ":");
		if(!strcmp(field, "Name"))
		{
			field = strtok(NULL, ":");
			printf("Name : %s",field);
		}
		if(!strcmp(field, "State"))
		{
			field = strtok(NULL, ":");
			printf("State : %s",field);
		}
		if(!strcmp(field, "Pid"))
		{
			field = strtok(NULL, ":");
			printf("Pid : %s",field);
		}
		if(!strcmp(field, "VmSize"))
		{
			field = strtok(NULL, ":");
			printf("Virtual Memory : %s",field);
		}
	}

	fclose(fp);	

	strcat(path, "/exe");
    
    if(!flag)
    	ret = readlink("/proc/self/exe", exe, sizeof(exe)-1);
    else
    	ret = readlink(path, exe, sizeof(exe)-1);
    
    if(ret == -1) 
    {
        printf("Executable path doesn't exist\n");
        return;
    }

    exe[ret] = '\0';
    printf("Executable path: %s\n",exe);

	return;
}

void execute_nightswatch(char** command)
{
	struct termios initial_settings, new_settings;
	char ch, l[MAX_SIZE];

	if(command[1]==NULL || strcmp(command[1], "-n") || command[2]==NULL || command[3]==NULL || (strcmp(command[3], "dirty") && strcmp(command[3], "interrupt")) || command[4]!=NULL)
	{
		fprintf(stderr, "Invalid syntax\n");
		return;
	}

	if(!check_integer(command[2]))
	{
		fprintf(stderr, "Given time interval is not a valid integer\n");
		return;
	}

	int interval = string_to_integer(command[2]);

	tcgetattr(0,&initial_settings);
 
  	new_settings = initial_settings;
  	new_settings.c_lflag &= ~ICANON;
  	new_settings.c_lflag &= ~ECHO;
  	new_settings.c_lflag &= ~ISIG;
  	new_settings.c_cc[VMIN] = 0;
  	new_settings.c_cc[VTIME] = 0;
 
 	tcsetattr(0, TCSANOW, &new_settings);

 	if(!strcmp(command[3], "dirty"))
 	{
		while(1)
		{
			FILE* fp = fopen("/proc/meminfo", "r");

			while(fgets(l, sizeof l, fp))
			{
				char* field = strtok(l, ":");
				if(!strcmp(field, "Dirty"))
				{
					field = strtok(NULL, ":");
					printf("%s\n",field);
				}
			}
			ch = getchar();
			
			if(ch!=EOF && ch=='q')
				break;
			
			sleep(interval);
		}
	}

	else if(!strcmp(command[3], "interrupt"))
 	{
 		int flag = 0, cpu_count;
		while(1)
		{
			FILE* fp = fopen("/proc/interrupts", "r");

			while(fgets(l, sizeof l, fp))
			{
				char* field = strtok(l, ":");
				if(!strcmp(field, "   1"))
				{
					field = strtok(NULL, ":");
					cpu_count = modify(field);
					if(!flag)
					{
						print_cpu(cpu_count);
						flag = 1;
					}
					printf("%s\n",field);
				}
			}
			ch = getchar();
			
			if(ch!=EOF && ch=='q')
				break;

			sleep(interval);
		}
	}

	tcsetattr(0, TCSANOW, &initial_settings);

	return;
}

void execute_setenv(char** command)
{
	if (command[1] == NULL)
		fprintf(stderr, "No input variable\n");

	else if (command[2] == NULL)
	{
		if(setenv(command[1], "", 1) == -1)
			fprintf(stderr, "Error!!\n");
	}

	else if (command[3] == NULL)
	{
		if(setenv(command[1], command[2], 1) == -1)
			fprintf(stderr, "Error!!\n");
	}

	else
		fprintf(stderr, "More than expected command line arguments!!");

	return;
}

void execute_unsetenv(char** command)
{
	if (command[1] == NULL)
		fprintf(stderr, "No input variable!!\n");

	else if (command[2] == NULL)
		unsetenv(command[1]) ;

	else
		fprintf(stderr, "More than expected command line arguments!!\n");

	return;
}

void execute_jobs(char** command)
{
	if(command[1] != NULL)
	{
		fprintf(stderr, "Invalid syntax!!\n");
		return;
	}

	int i;

	for(i=0;i<pointer;i++)	
	{
		printf("[%d]   ", i+1);
		if(status[i] == 1)
			printf("Running   ");
		else if(status[i] == -1)
			printf("Stopped   ");
		printf("pid[%d]\n",jobs[i]);
	}

	return;
}

void execute_kjob(char** command)
{
	if(command[1] == NULL)
	{
		fprintf(stderr, "Invalid syntax!!\n");
		return;
	}

	if(command[2] == NULL)
	{
		fprintf(stderr, "Invalid syntax!!\n");	
		return;
	}

	int jobno = atoi(command[1]);

	if(jobno > pointer)
	{
		fprintf(stderr, "Invalid job number!!\n");
		return;
	}

	kill(jobs[jobno-1], atoi(command[2]));

	if(atoi(command[2]) == 9)
		Remove(jobs[jobno-1]);

	return;
}

void execute_fg(char** command)
{
	if(command[1] == NULL)
	{
		fprintf(stderr, "Invalid syntax!!\n");
		return;
	}

	if(command[2] != NULL)
	{
		fprintf(stderr, "Invalid syntax!!\n");
		return;
	}

	int jobno = atoi(command[1]);

	if(jobno>pointer || jobno<1)
	{
		fprintf(stderr, "Job number doesn't exist!!\n");
		return;
	}

	int pid = jobs[jobno-1];

	kill(pid, SIGCONT);

	// if(tcsetpgrp(0, pid) == -1)
	// {
	// 	fprintf(stderr, "Error in changing process group!!\n");
	// 	return;
	// }

	Remove(pid);

	siginfo_t *infop;

	waitid(P_PID, pid, infop, (WUNTRACED | WNOWAIT));

	return;
}

void execute_bg(char** command)
{
	if(command[1] == NULL)
	{
		fprintf(stderr, "Invalid syntax!!\n");
		return;
	}

	if(command[2] != NULL)
	{
		fprintf(stderr, "Invalid syntax!!\n");
		return;
	}

	int jobno = atoi(command[1]);

	if(jobno<1 || jobno>pointer)
	{
		fprintf(stderr, "Job number doesn't exist!!\n");
		return;
	}

	int pid = jobs[jobno-1];

	status[jobno-1] = 1;

	kill(pid, SIGCONT);

	return;
}

void execute_overkill(char** command)
{
	if(command[1] != NULL)
	{
		fprintf(stderr, "Invalid syntax!!\n");
		return;
	}

	int i;

	for(i=0;i<pointer;i++)
	{
		kill(jobs[i], 9);
		Remove(jobs[i]);
	}

	return;
}
