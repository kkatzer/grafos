#include <graphviz/cgraph.h>
#include <stdio.h>
#include <stdlib.h>
#include "grafo.h"

struct grafo {
  int ponderado;

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
  return (unsigned int)agnnodes(g->grafo);
}

unsigned int n_arestas(grafo g){
  return (unsigned int)agnedges(g->grafo);
}

struct vertice {
  Agnode_t *vertice;
};

char *nome_vertice(vertice v){
  return agnameof(v->vertice);
}

grafo le_grafo(FILE *input){
	grafo g = malloc(sizeof(struct grafo));
	Agraph_t *h = agread(input, NULL);
	g->grafo = h;

	return g;
}

int destroi_grafo(void *g) {
	grafo *f = (void *)g;
  int ok = 1;
  ok &= agclose((*f)->grafo);
  if (ok == 1){
    free(g);
  }
  if (g == NULL){
  	ok &= 1;
  }
  return ok;
}

grafo escreve_grafo(FILE *output, grafo g){
  if (agwrite(g->grafo, output)){
  	return g;
  } else {
  	return NULL;
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
    return (unsigned int)agdegree(g->grafo,v->vertice,TRUE,TRUE);
  } else if (direcao < 0) {
    return (unsigned int)agdegree(g->grafo,v->vertice,TRUE,FALSE);
  } else {
    return (unsigned int)agdegree(g->grafo,v->vertice,FALSE,TRUE);
  }
}

int clique(lista l, grafo g){
  int is = 1;
  no no1 = primeiro_no(l);
  no no2 = proximo_no(no1);
  vertice *v1, *v2;
  while(no1 != NULL){
    while(no2 != NULL){
    	v1 = (void *)conteudo(no1);
    	v2 = (void *)conteudo(no2);
      if (agedge(g->grafo, (*v1)->vertice, (*v2)->vertice, NULL, FALSE) == NULL){
        is = 0;
      }
      no2 = proximo_no(no2);
    }
    no1 = proximo_no(no1);
    no2 = proximo_no(no1);
  }
  return is;
}

int simplicial(vertice v, grafo g){
  lista vz = vizinhanca(v,0,g);
  return clique(vz,g);
}

int cordal(grafo g){
	grafo h;
	h = g;
	if (h == g){
	  return 1;
	}
	return 1;
}
