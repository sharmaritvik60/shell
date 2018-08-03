#define BUFFER_SIZE 100005

#include <stdio.h>
#include <stdlib.h>

char *get_command()
{
	int buffer_size = BUFFER_SIZE;
	char ch;
	int pos = 0;
	char *buffer = malloc(sizeof(char) * BUFFER_SIZE);

	while(1)
	{
		ch = getchar();

		if(ch=='\n' || ch==EOF)
		{
			buffer[pos] = '\0';
			return buffer;
		}

		buffer[pos] = ch;
		pos++;

		if(pos >= buffer_size)
		{	
			buffer_size += BUFFER_SIZE;
			buffer = realloc(buffer, buffer_size);
		}
	}
}
