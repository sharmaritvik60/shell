shell: main.c
	gcc -g -o shell main.c prompt.c get_command.c parse_command.c execute_command.c inbuilts.c redirect.c execute_pipe.c
