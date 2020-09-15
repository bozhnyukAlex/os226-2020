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

int main(int argc, char *argv[]) {
	char curString[SIZE];
	char* command;
	char* commandSave = NULL;
	char* cwordSave = NULL;
	
	while (fgets(curString, sizeof(curString), stdin)) {
		command = strtok_r(curString, ";\n", &commandSave); // считываем отдельную команду со всеми аргументами до ; или перехода

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
	return 0;
}
