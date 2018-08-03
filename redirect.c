#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/utsname.h>
#include <string.h>
#include <fcntl.h>
#include <pwd.h>

void redirect (char ** args)
{
	int fd1, fd2, fd3;
	char in[100];
	char out[100];
	char outdir[100];

	int flag1, flag2, flag3;
	flag1 = flag2 = flag3 = 0;

	char * q = args[0];
	int c=0;

	while(q!=NULL)
	{
	 	if (strncmp(q,"<",1)==0)
	  	{
	    	args[c]=NULL;
		    flag1=1;
		    strcpy(in,args[c+1]);	
		}

		if((strncmp(q,">",1)==0))
	  	{
		   	args[c]=NULL;
		   	flag2=1;
		   	strcpy(out,args[c+1]);
		}

	  	if(strncmp(q,">>",2)==0)
	  	{
	    	args[c]=NULL;
	    	flag3=1;
	    	strcpy(outdir,args[c+1]);
	  	}
	  
	  c++;
	  q = args[c];
	}


	if(flag1==0 && flag2==0 && flag3==0)
	  return;

	if(flag1==1)
	{
		fd1 = open(in, O_RDONLY,0);

	  //if error
	  	if(fd1 < 0)
	  	{
	    	perror("Could not open the input file");
	    	exit(0);
	  	}

	  	dup2(fd1,0);
	  	close(fd1);
	}

	if(flag2==1)
	{
	  	fd2 = open (out, O_WRONLY | O_CREAT, 0644);
	  	if(fd2<0)
	  	{
	    	perror("Could not create the output file");
	    	exit(0);
		}

	  	dup2(fd2,1);
		close(fd2);
	}

	if(flag3==1)
	{
	  	fd3 = open (outdir, O_WRONLY | O_CREAT | O_APPEND , 0644);
	  	if(fd3<0)
	  	{
	    	perror("Could not create the output file");
	    	exit(0);
	  	}

	  	dup2(fd3,1);
	  	close(fd3);
	}

	return;
}
