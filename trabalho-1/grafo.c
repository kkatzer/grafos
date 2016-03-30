#include <graphviz/cgraph.h>
#include <stdio.h>
#include "grafo.h"

struct grafo {
  int ponderado;

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
  return tamanho_lista(g->vertices);
}

unsigned int n_arestas(grafo g){
  return tamanho_lista(g->arestas);
}

struct vertice {
  Agnode_t *vertice;
}

char *nome_vertice(vertice v){
  return agnameof(v->vertice);
}

grafo le_grafo(FILE *input){
  grafo g;
  g->grafo = agread(input, NULL);

  return g;
}

int destroi_grafo(void *g) {
  int ok = 1;
  agclose(g->grafo);
  destroi_lista(g->vertices, agclose);
  destroi_lista(g->arestas, agclose);
  ok &= g->grafo & g->vertices & g->arestas;
  if (ok == 1){
    free(g);
  }
  ok &= g;
  return ok;
}

grafo escreve_grafo(FILE *output, grafo g){
  Agraph_t *h;
  h = agwrite(g->grafo, output);

  if (h == NULL) {
    return -1;
  } else {
    return 0;
  }
}

grafo copia_grafo(grafo g){
  grafo h;
  h = g;
  return h;
}

lista vizinhanca(vertice v, int direcao, grafo g){
  lista vz;
  Agedge_t *e;
  vz = constroi_lista();
  if (direcao == 0) {
    for (e = agfstedge(g->grafo,v->vertice); e; e = agnxtedge(g->grafo,e,v->vertice)){
      insere_lista(e,vz);
    }
  } else if (direcao < 0) {
    for (e = agfstin(g->grafo,v->vertice); e; e = agnxtin(g->grafo,e)) {
      insere_lista(e,vz);
    }
  } else {
    for (e = agfstout(g->grafo,v->vertice); e; e = agnxtout(g->grafo,e)) {
      insere_lista(e,vz);
    }
  }
  return vz;
}

unsigned int grau(vertice v, int direcao, grafo g){
  if (direcao == 0){
    return agdegree(g->grafo,v->vertice,TRUE,TRUE);
  } else if (direcao < 0) {
    return agdegree(g->grafo,v->vertice,TRUE,FALSE);
  } else {
    return agdegree(g->grafo,v->vertice,FALSE,TRUE);
  }
}

int clique(lista l, grafo g){
  int is = 1;
  no no1 = primeiro_no(l);
  no no2 = proximo_no(no1);
  while(no1 != NULL){
    while(no2 != NULL){
      if (agedge(g->grafo, conteudo(no1)->vertice, conteudo(no2)->vertice, NULL, FALSE) == NULL){
        is = 0;
      }
      no2 = proximo_no(no2);
    }
    no1 = proximo_no(no1);
    no2 = proximo_no(no1);
  }
}

int simplicial(vertice v, grafo g){
  lista vz = vizinhanca(v,0,g);
  return clique(vz,g);
}

int cordal(grafo g){
  return 1;
}
