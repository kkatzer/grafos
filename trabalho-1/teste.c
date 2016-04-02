#include <stdio.h>
#include <stdlib.h>
#include "grafo.h"

//------------------------------------------------------------------------------

int main(void) {

  grafo h = le_grafo(stdin);
  grafo g = copia_grafo(h);

  if ( !g )

    return 1;

  printf("nome: %s\n", nome_grafo(g));
  printf("%sdirecionado\n", direcionado(g) ? "" : "não ");
  printf("%sponderado\n", ponderado(g) ? "" : "não ");
  printf("%d vértices\n", n_vertices(g));
  printf("%d arestas\n", n_arestas(g));

  escreve_grafo(stdout, g);

  return ! destroi_grafo(g);
  // return 1;
}
