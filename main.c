#include <stdio.h>
#include <stdlib.h>
#include "geracodigo.h"

void printInstruction(unsigned char first[], int number);

int main(int argc, char *argv[]) {
  FILE *myfp;
  unsigned char codigo[600];
  funcp funcaoSB;
  int res;
  int arg1 = 0, arg2 = 0;

  /* Abre o arquivo fonte */
  if ((myfp = fopen(argv[1], "r")) == NULL) {
    perror("Falha na abertura do arquivo fonte");
    exit(1);
  }
  if (argc > 2) {
    arg1 = atoi(argv[2]);
  } if (argc > 3) {
    arg2 = atoi(argv[3]);
  }
  /* compila a função SB */
  funcaoSB = geraCodigo(myfp, codigo);
  fclose(myfp);

  printInstruction((unsigned char*)codigo, 80);
  /* chama a função */
  //res = (*funcaoSB) (arg1, arg2);  /* passando parâmetro apropriados */
  //printf("resultado: %d\n", res);
}
