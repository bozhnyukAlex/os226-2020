#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define SYMBOL_COUNT 11
#define STATES_COUNT 15


enum symbols {SP = 0, LF, SEM, E, C, H, O, R, T, D, OTHER};

enum states {START = 0, E_ECHO, C_ECHO, H_ECHO, O_ECHO, SP_ECHO, ARG_ECHO, R_RET, E1_RET, T_RET, C_RET, O_RET, D_RET, E2_RET, ERROR};

enum states FSM_Table[STATES_COUNT][SYMBOL_COUNT] = {
	[START][SP] = START, [START][LF] = START, [START][SEM] = START, [START][E] = E_ECHO,
	[START][C] = ERROR, [START][H] = ERROR, [START][O] = ERROR, [START][R] = R_RET,
	[START][T] = ERROR, [START][D] = ERROR, [START][OTHER] = ERROR,
	//-----
	[E_ECHO][SP] = ERROR, [E_ECHO][LF] = ERROR, [E_ECHO][SEM] = ERROR, [E_ECHO][E] = ERROR,
	[E_ECHO][C] = C_ECHO, [E_ECHO][H] = ERROR, [E_ECHO][O] = ERROR, [E_ECHO][R] = ERROR,
	[E_ECHO][T] = ERROR, [E_ECHO][D] = ERROR, [E_ECHO][OTHER] = ERROR,
	//-----
	[C_ECHO][SP] = ERROR, [C_ECHO][LF] = ERROR, [C_ECHO][SEM] = ERROR, [C_ECHO][E] = ERROR,
	[C_ECHO][C] = ERROR, [C_ECHO][H] = H_ECHO, [C_ECHO][O] = ERROR, [C_ECHO][R] = ERROR,
	[C_ECHO][T] = ERROR, [C_ECHO][D] = ERROR, [C_ECHO][OTHER] = ERROR,
	//-----
	[H_ECHO][SP] = ERROR, [H_ECHO][LF] = ERROR, [H_ECHO][SEM] = ERROR, [H_ECHO][E] = ERROR,
	[H_ECHO][C] = ERROR, [H_ECHO][H] = ERROR, [H_ECHO][O] = O_ECHO, [H_ECHO][R] = ERROR,
	[H_ECHO][T] = ERROR, [H_ECHO][D] = ERROR, [H_ECHO][OTHER] = ERROR,
	//-----
	[O_ECHO][SP] = SP_ECHO, [O_ECHO][LF] = START, [O_ECHO][SEM] = START, [O_ECHO][E] = ERROR,
	[O_ECHO][C] = ERROR, [O_ECHO][H] = ERROR, [O_ECHO][O] = ERROR, [O_ECHO][R] = ERROR,
	[O_ECHO][T] = ERROR, [O_ECHO][D] = ERROR, [O_ECHO][OTHER] = ERROR,
	//----
	[SP_ECHO][SP] = SP_ECHO, [SP_ECHO][LF] = START, [SP_ECHO][SEM] = START, [SP_ECHO][E] = ARG_ECHO,
	[SP_ECHO][C] = ARG_ECHO, [SP_ECHO][H] = ARG_ECHO, [SP_ECHO][O] = ARG_ECHO, [SP_ECHO][R] = ARG_ECHO,
	[SP_ECHO][T] = ARG_ECHO, [SP_ECHO][D] = ARG_ECHO, [SP_ECHO][OTHER] = ARG_ECHO,
	//---
	[ARG_ECHO][SP] = SP_ECHO, [ARG_ECHO][LF] = START, [ARG_ECHO][SEM] = START, [ARG_ECHO][E] = ARG_ECHO,
	[ARG_ECHO][C] = ARG_ECHO, [ARG_ECHO][H] = ARG_ECHO, [ARG_ECHO][O] = ARG_ECHO, [ARG_ECHO][R] = ARG_ECHO,
 	[ARG_ECHO][T] = ARG_ECHO, [ARG_ECHO][D] = ARG_ECHO, [ARG_ECHO][OTHER] = ARG_ECHO,
	//---
	[R_RET][SP] = ERROR, [R_RET][LF] = ERROR, [R_RET][SEM] = ERROR, [R_RET][E] = E1_RET,
	[R_RET][C] = ERROR, [R_RET][H] = ERROR, [R_RET][O] = ERROR, [R_RET][R] = ERROR, 
	[R_RET][T] = ERROR, [R_RET][D] = ERROR, [R_RET][OTHER] = ERROR,
	//--- 
	[E1_RET][SP] = ERROR, [E1_RET][LF] = ERROR, [E1_RET][SEM] = ERROR, [E1_RET][E] = ERROR,
	[E1_RET][C] = ERROR, [E1_RET][H] = ERROR, [E1_RET][O] = ERROR, [E1_RET][R] = ERROR,
	[E1_RET][T] = T_RET, [E1_RET][D] = ERROR, [E1_RET][OTHER] = ERROR,
	//---
	[T_RET][SP] = ERROR, [T_RET][LF] = ERROR, [T_RET][SEM] = ERROR, [T_RET][E] = ERROR,
	[T_RET][C] = C_RET, [T_RET][H] = ERROR, [T_RET][O] = ERROR, [T_RET][R] = ERROR,
	[T_RET][T] = ERROR, [T_RET][D] = ERROR, [T_RET][OTHER] = ERROR,
	//---
	[C_RET][SP] = ERROR, [C_RET][LF] = ERROR, [C_RET][SEM] = ERROR, [C_RET][E] = ERROR,
	[C_RET][C] = ERROR, [C_RET][H] = ERROR, [C_RET][O] = O_RET, [C_RET][R] = ERROR,
	[C_RET][T] = ERROR, [C_RET][D] = ERROR, [C_RET][OTHER] = ERROR,
	//---
	[O_RET][SP] = ERROR, [O_RET][LF] = ERROR, [O_RET][SEM] = ERROR, [O_RET][E] = ERROR,
	[O_RET][C] = ERROR, [O_RET][H] = ERROR, [O_RET][O] = ERROR, [O_RET][R] = ERROR, 
	[O_RET][T] = ERROR, [O_RET][D] = D_RET,	[O_RET][OTHER] = ERROR,
	//---
	[D_RET][SP] = ERROR, [D_RET][LF] = ERROR, [D_RET][SEM] = ERROR, [D_RET][E] = E2_RET,
	[D_RET][C] = ERROR, [D_RET][H] = ERROR, [D_RET][O] = ERROR, [D_RET][R] = ERROR,
	[D_RET][T] = ERROR, [D_RET][D] = ERROR, [D_RET][OTHER] = ERROR,
	//---
	[E2_RET][SP] = E2_RET, [E2_RET][LF] = START, [E2_RET][SEM] = START, [E2_RET][E] = ERROR,
	[E2_RET][C] = ERROR, [E2_RET][H] = ERROR, [E2_RET][O] = ERROR, [E2_RET][R] = ERROR,
	[E2_RET][T] = ERROR, [E2_RET][D] = ERROR, [E2_RET][OTHER] = ERROR
};

enum symbols translate(char ch) {
	switch (ch) {
		case ' ':
			return SP;
		case '\n':
			return LF;
		case ';':
			return SEM;
		case 'e':
			return E;
		case 'c':
			return C;
		case 'h':
			return H;
		case 'o':
			return O;
		case 'r':
			return R;
		case 't':
			return T;
		case 'd':
			return D;
		default:
			return OTHER;
	}
}


int echo(int argc, char *argv[]) {
	for (int i = 1; i < argc; ++i) {
		printf("%s%c", argv[i], i == argc - 1 ? '\n' : ' ');
	}
	return argc - 1;
}

int retcode(int argc, char *argv[]) {
}

int main(int argc, char *argv[]) {
	FILE* input;
	if ((input = fopen("input.txt", "rt")) == NULL) {
		printf("Problem with opening file\n");
		return -1;
	}
	char curCh;
	while ((curCh = fgetc(input)) != EOF) {
		enum symbols sym = translate(curCh);
		
	}
	return 0;
}
