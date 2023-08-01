#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include "main.h"


void rash_loop() {
	char *line; //pointer to the char that will be the command on the line
	char **args; //pointer to pointer of arguments
	int status;
	do {
		printf("$ ");
		line = read_line();
		args = parse_line(line);
		status = execute(args);

		free(line);
		free(args);
	} while (status);
}

#define RASH_BUFSIZE 1024 //length of text a user can input into a shell

char *read_line() { //returns a pointer to a char (the input text the user gives)
	int bufsize = RASH_BUFSIZE;
	int position = 0;
	char *buffer = malloc(sizeof(char) * bufsize);
	int curr; //current character;

	if (!buffer) {
		fprintf(stderr, "RASH: allocation error\n");
		exit(EXIT_FAILURE);
	}

	while (1) {
		curr = getchar(); //reads a character

		if (curr == EOF || curr == '\n') {
			buffer[position] = '\0'; //buffer position should reset
			return buffer;
		}
		else {
			buffer[position] = curr; //add the next character into the buffer
		}
		position++;

		if (position >= bufsize) { //if command is bigger than allocated memory
			bufsize += RASH_BUFSIZE;
			buffer = realloc(buffer, bufsize);
			if (!buffer) {
				fprintf(stderr, "RASH: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}
	}
}

#define RASH_TOK_BUFSIZE 64 //size a token can be
#define RASH_TOK_DELIM " \t\r\n\a" //these are the characters that can separate tokens from one another to execute
char **parse_line(char *line) { //will return pointer to the pointer of the input the user gave.This way we can access both the memory address of the input and the value itself

	int bufsize = RASH_BUFSIZE;
	int position = 0;
	char **tokens = malloc(sizeof(char*) * bufsize);
	char *token;

	if (!tokens) {
		fprintf(stderr, "RASH: allocation error\n");
		exit(EXIT_FAILURE);
	}

	token = strtok(line, RASH_TOK_DELIM); //current token we are on. strtok will return a pointer to this token

	while (token != NULL) {
		tokens[position] = token; //store each pointer into an array of character pointers
		position++;

		if (position >= bufsize) {
			bufsize += RASH_TOK_BUFSIZE;
			tokens = realloc(tokens, bufsize);

			if (!tokens) {
				fprintf(stderr, "RASH: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}
		token = strtok(NULL, RASH_TOK_DELIM); //continuing down the line and splitting whenever we reach one of the defined delimiters.
	}

	tokens[position] = NULL;
	return tokens;

}

int launch(char **args) { //takes in the array of tokens to execute
	pid_t pid, wpid; //creating two variables to store process IDs
			 //pid is used for the child process. wpid will be used for parent
	int status;

	/*
	fork will return both the parent and child processes. 0 will execute the child
	1 will execute the parent
	 */
	pid = fork(); //will cause two processes to run concurrently. Child and Parent

	if (pid == 0) { //Execute Child Process
		if (execvp(args[0], args) == -1) { //executes args[0] which is the name of the command to run. args is the complete list of arguments to execute alone with the command
			perror("RASH"); //if the command fails
		}
	}
	else if (pid < 0) { //some error happened
		perror("RASH");
	}
	else { //execute parent process
		do {
			wpid = waitpid(pid, &status, WUNTRACED); //make sure the child process finishes before we execute the parent process
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	return 1; //once it returns this, we are prompting the user for input again because we have executed the child function.
		  // we haven't done anything with the parent but that will then be forked in the next iteration of our user inputting a command. So on, so forth.
}

int num_builtins() {
	return sizeof(builtin_cmd) / sizeof(char *);
}

int rash_cd(char **args) {
	if (args[1] == NULL) {
		fprintf(stderr, "RASH: expected argument to \"cd\"\n");
	}
	else {
		if (chdir(args[1]) != 0) {
			perror("RASH");
		}
	}
	return 1;
}

int rash_help(char **args) {
	int i;

	printf("RASH Shell\n");
	printf("Type program names and arguments, and hit enter.\n");

	return 1;
}

int rash_exit(char **args) {
	return 0;
}

int execute(char **args) {
	int i;

	if (args[0] == NULL) {
		return 1;
	}

	for (i = 0; i < num_builtins(); i++) {
		if (strcmp(args[0], builtin_cmd[i]) == 0) {
			return (*builtin_func[i])(args);
		}
	}
	return launch(args);
}

int main(int argc, char **argv) {
	rash_loop();

	return EXIT_SUCCESS;
}
