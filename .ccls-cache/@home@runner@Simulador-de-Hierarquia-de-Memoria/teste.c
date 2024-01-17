#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main() {
  int matriz[100];
  int i;

  // inicializa a matriz com valores aleatórios
  for (i = 0; i < 100; i++) {
    matriz[i] = rand();
  }

  // acessa a matriz usando um endereço virtual
  long int endereco_virtual = 0x12345678;
  int* endereco_fisico = (int*)(intptr_t)endereco_virtual;
  printf("Valor na posicao %ld da matriz: %d\n", endereco_virtual, *endereco_fisico);

  return 0;
}
