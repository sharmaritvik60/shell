#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int check_less(char **s)
{
  int i=0;
  while(s[i]!=0)
  {
    if((strcmp(s[i],"<")==0))
      return 1;
    i++;
  }
  return 0;
}

int check_great(char **s)
{
  int i=0;
  while(s[i]!=0)
  {
    if((strcmp(s[i],">")==0))
      return 1;
    i++;
  }
  return 0;
}

void  loop_pipe(char ***cmd) 
{
  int   p[2];
  pid_t pid;
  int   fd_in = 0;

  while (*cmd != NULL)
  {
    int cl=0,i=0,l=0,cg=0;
    pipe(p);
    if ((pid = fork()) == -1)
    {
      exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
      dup2(fd_in, 0); //change the input according to the old one 
      if (*(cmd + 1) != NULL)
        dup2(p[1], 1);

      close(p[0]);
      cl=check_less(*cmd);
      if(cl)
      {
        i=0;
        while( (strcmp((*cmd)[i],"<") !=0 ) && (strcmp((*cmd)[i],">") !=0 ) )
          i++;
        i++;
        l=i;
        while( (*cmd)[i]!=NULL )
        {
          strcpy((*cmd)[i-1],(*cmd)[i]);
          i++;
        }
        (*cmd)[i-1]=NULL;
        int in;
        i-=2;
        in=open((*cmd)[i], O_RDONLY);
        if(in<0)
        {
          printf("%s :File doesn't exist\n",(*cmd)[i]);
          return ;
        }
        else
          dup2(in,0);
        char *arg[l];
        for(i=0;i<l-1;i++)
          arg[i]=(*cmd)[i];
        arg[l-1]=NULL;
        execvp(arg[0],arg);

      }
      cg=check_great(*cmd);
  //    printf("%d\n",cg);
    if(cg)
      {

        i=0;
        while( (strcmp((*cmd)[i],"<") !=0 ) && (strcmp((*cmd)[i],">") !=0 ) )
          i++;
        i++;
        l=i;
        while( (*cmd)[i]!=NULL )
        {
          strcpy((*cmd)[i-1],(*cmd)[i]);
          i++;
        }
        (*cmd)[i-1]=NULL;
        i=0;
        while((*cmd)[i]!=NULL)
        {
      //    printf("%s\n",(*cmd)[i]);
          i++;
        }
        i-=1;
      //  printf("i=%d\n",i);
        char *arg[3];
        arg[2]=NULL;
        int out=open((*cmd)[i],O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR );

        int j;
        for(j=0;j<2;j++)
        {
          arg[j]=(*cmd)[j];
      //    printf("%s\n",arg[j]);
        }
        dup2(out,1);
        execvp(arg[0],arg);
        
      }
      else execvp((*cmd)[0], *cmd);
      exit(EXIT_FAILURE);
    }
    else
    {
      wait(NULL);
      close(p[1]);
      fd_in = p[0]; //save the input for the next command
      cmd++;
    }
  }
}
void parse_pipe(char* s)
{
  char *end_str;
  char *token = strtok_r(s, "|", &end_str);
  int i=0,j=0;
  char ***com = (char ***) malloc(sizeof(char**) * 100); 

  while (token != NULL)
  {
    j=0;
    com[i] = (char **) malloc(sizeof(char*) * 100); 
    char *end_token;
    char *token2 = strtok_r(token, " \t\n\r", &end_token);
    while (token2 != NULL)
    {
      com[i][j] = (char *) malloc(sizeof(char) * 100);
      strcpy(com[i][j],token2);
      token2 = strtok_r(NULL, " \t\r\n", &end_token);
      j++;
    }
    com[i][j]=NULL;
    token = strtok_r(NULL, "|", &end_str);
    i++;
  }
  com[i]=NULL;
  i=0;
  loop_pipe(com);
}