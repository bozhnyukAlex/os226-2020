#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define SIZE 256

int rcode = 0;


int echo(int argc, char *argv[]) {
	for (int i = 1; i < argc; ++i) {
		printf("%s%c", argv[i], i == argc - 1 ? '\n' : ' ');
	}
	rcode = argc - 1;
	return argc - 1;
}

int retcode(int argc, char *argv[]) {
	printf("%d\n", rcode);
}

void printError() {
	printf("Input error!\n");
}

void parseLine(char line[], int argc, char* argv[]) {
	char* commandSave = NULL;
	char* cwordSave = NULL;
	char* command = strtok_r(line, ";\n", &commandSave); // считываем отдельную команду со всеми аргументами до ; или перехода

		while (command != NULL) { // now parsing command
			argc = 0;
			char* curWord = strtok_r(command, " ", &cwordSave);

			while (curWord != NULL) {
				argv[argc++] = curWord;
				curWord = strtok_r(NULL, " ", &cwordSave); //argv[0] - command, argv[i], i > 0 - arguments
			}

			if (strcmp(argv[0], "echo") == 0) {
				echo(argc, argv);
			}
			else if (strcmp(argv[0], "retcode") == 0) {
				retcode(argc, argv);
			}
			else {
				printError();
			}

			command = strtok_r(NULL, ";\n", &commandSave);
	}
}

int main(int argc, char *argv[]) {
	char line[SIZE];
	while (fgets(line, sizeof(line), stdin)) {
		parseLine(line, argc, argv);
	}
	return 0;
}
