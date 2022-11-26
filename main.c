#include "geracodigo.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

void printInstruction(unsigned char first[], int number);

static void error (const char *msg, int line) {
    fprintf(stderr, "erro %s na linha %d\n", msg, line);
    exit(EXIT_FAILURE);
}

int main (int argc, char *argv[]) {
    FILE *myfp;
    funcp f;
    int arg1 = 0, arg2 = 0;
    
    if (argc != 2){
        fprintf(stderr, "uso do programa: ./main <nome do programa SB>\n");
        exit(1);
    }
    
    if(argc > 2) {
        arg1 = atoi(argv[2]);
    }
    if(argc > 3){
        arg2 = atoi(argv[3]);
    }
    
    unsigned char* codigo = (unsigned char*)( malloc( 300 * sizeof(unsigned char) ) );
    
    if ((myfp = fopen (argv[1], "r")) == NULL) {
        perror ("nao conseguiu abrir arquivo!");
        exit(1);
    }
    
    f = geraCodigo(myfp, codigo);
    printf("resultado: %d\n", (*f)(arg1, arg2));
}
