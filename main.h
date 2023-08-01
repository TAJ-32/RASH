#ifndef MAIN_H
#define MAIN_H


void rash_loop();
char *read_line();
char **parse_line(char *line);
int execute(char **args);
int launch(char **args);
int num_builtins();


int rash_cd(char **args);
int rash_help(char **args);
int rash_exit(char **args);

char *builtin_cmd[] = {
	"cd",
	"help",
	"exit"
};

int (*builtin_func[]) (char **) = {
	&rash_cd,
	&rash_help,
	&rash_exit
};

#endif
