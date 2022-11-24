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
    
    if (argc != 2){
        fprintf(stderr, "uso do programa: ./main <nome do programa SB>\n");
        exit(1);
    }
    
    unsigned char* codigo = (unsigned char*)( malloc( 300 * sizeof(unsigned char) ) );
    
    if ((myfp = fopen (argv[1], "r")) == NULL) {
        perror ("nao conseguiu abrir arquivo!");
        exit(1);
    }
    
    codigo = geraCodigo(myfp, codigo);
    printInstruction(codigo, 80);
}
