#ifndef OPERATIONS_H
#define OPERATIONS_H

// Bibliotecas Locais
#include "../headers/LDC.h"
#include "../headers/AVL.h"

void operacaoA (AVL* localidades, FILE* saida, char nome[], int TExpected);

void operacaoB (LDC *consultas, FILE *saida, int TExpected);

#endif // OPERATIONS_H
