#include <graphviz/cgraph.h>
#include <stdio.h>
#include "grafo.h"

typedef int bool;
#define true 1
#define false 0

struct grafo {
  bool ponderado;

  lista vertices;
  lista arestas;

  Agraph_t *grafo;
};

char *nome_grafo(grafo g){
  return agnameof(g->grafo);
}

int direcionado(grafo g){
  return agisdirected(g->grafo);
}

int ponderado(grafo g){
  return g->ponderado;
}

unsigned int n_vertices(grafo g){
  g->vertices->tamanho;
}

unsigned int n_arestas(grafo g){
  g->arestas->tamanho;
}

struct vertice {
  char* nome;

  Agnode_t vertice;
};

char *nome_vertice(vertice v){
  return v.nome;
}

grafo le_grafo(FILE *input){
  grafo g;
  g.grafo = agread(input, NULL);
}
