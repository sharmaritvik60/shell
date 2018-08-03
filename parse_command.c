#define BUFFER_SIZE 64

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* trim(char *command)
{
	int buffer_size = BUFFER_SIZE;
	int i=1, j=0;
	char *buffer = malloc(sizeof(char) * BUFFER_SIZE);

	while(1)
	{
		if(*(command+i-1)  == ' ')
			i++;
		else
			break;
	}
	
	while(1)
	{
		if(*(command+i)!=' ' || *(command+i-1)!=' ')
		{
			buffer[j] = *(command+i-1);
			j++;
		}
		if(*(command+i) == '\0')
		{
			buffer[j] = '\0';
			return buffer;
		}
		i++;
		if(j >= buffer_size)
		{	
			buffer_size += BUFFER_SIZE;
			buffer = realloc(buffer, buffer_size);
		}
	}
}

char** parse_command(char* command, const char token_delimiter)
{
	int buffer_size = BUFFER_SIZE;
	char** tokens = malloc(buffer_size * sizeof(char*));
	char* token;
	int pos = 0;
	char* trimmed_command = trim(command);

	token = strtok(trimmed_command, &token_delimiter);
	while(token != NULL)
	{
		tokens[pos] = token;
		pos++;
		
		if(pos >= buffer_size)
		{
			buffer_size += BUFFER_SIZE;
			tokens = realloc(tokens, buffer_size * sizeof(char*));
		}

		token = strtok(NULL, &token_delimiter);
	}

	tokens[pos] = NULL;
	return tokens;
}
