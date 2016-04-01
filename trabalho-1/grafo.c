#include <graphviz/cgraph.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grafo.h"

struct grafo {
  char *nome;
  int ponderado;
  int direcionado;

  lista vertices;
  lista arestas;
};

lista povoa_v_arestas(Agnode_t *v, Agraph_t *G);
lista povoa_vertices(grafo g, Agraph_t *G);
lista povoa_arestas(Agraph_t *G);

char *nome_grafo(grafo g){
  return g->nome;
}

int direcionado(grafo g){
  return g->direcionado;
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
  char *nome;
  unsigned int grau[2];

  lista vizinhanca[2];
  lista arestas;
};

char *nome_vertice(vertice v){
  return v->nome;
}

typedef struct aresta {
  char *head;
  char *tail;

  char *nome;
  char *peso;
} *aresta;

lista povoa_v_arestas(Agnode_t *v, Agraph_t *G){
  aresta a = NULL;
  char peso[] = "peso";
  lista arestas = constroi_lista();
  for (Agedge_t *e = agfstin(G,v); e; e = agnxtin(G,e)) {
    a = malloc(sizeof(struct aresta));
    a->head = agnameof(aghead(e));
    a->tail = agnameof(agtail(e));
    a->nome = agnameof(e);
    a->peso = agget(e,peso);
    if (insere_lista(a,arestas) == NULL){
      return NULL;
    }
  }
  return arestas;
}

lista povoa_vertices(grafo g, Agraph_t *G){
  lista vertices = constroi_lista();
  vertice v = malloc(sizeof(struct vertice));
  Agnode_t *n;
  Agedge_t *e;
  for (n = agfstnode(G); n; n = agnxtnode(G,n)){
    v = malloc(sizeof(struct vertice));
    v->nome = agnameof(n);
    //grau e vizinhanca
    v->vizinhanca[0] = constroi_lista();
    v->vizinhanca[1] = constroi_lista();
    if (g->direcionado) {
      v->grau[0] = (unsigned int)agdegree(G,n,TRUE,FALSE);
      v->grau[1] = (unsigned int)agdegree(G,n,FALSE,TRUE);
      for (e = agfstin(G,n); e; e = agnxtin(G,e)) {
        insere_lista(e->node,v->vizinhanca[0]);
      }
      for (e = agfstout(G,n); e; e = agnxtout(G,e)) {
        insere_lista(e->node,v->vizinhanca[1]);
      }
    } else {
      v->grau[0] = (unsigned int)agdegree(G,n,TRUE,TRUE);
      for (e = agfstedge(G,n); e; e = agnxtedge(G,e,n)){
        insere_lista(e->node,v->vizinhanca[0]);
      }
    }
    v->arestas = povoa_v_arestas(n,G);
    insere_lista(v,vertices);
  }
  return vertices;
}

lista povoa_arestas(Agraph_t *G){
  aresta a = NULL;
  char peso[] = "peso";
  lista arestas = constroi_lista();
  for (Agnode_t *n = agfstnode(G); n; n = agnxtnode(G,n)) {
    for (Agedge_t *e = agfstout(G,n); e; e = agnxtout(G,e)) {
      a = malloc(sizeof(struct aresta));
      a->head = agnameof(aghead(e));
      a->tail = agnameof(agtail(e));
      a->nome = agnameof(e);
      a->peso = agget(e,peso);
      if (insere_lista(a,arestas) == NULL){
        return NULL;
      }
    }
  }
  return arestas;
}

grafo le_grafo(FILE *input){
	grafo g = malloc(sizeof(struct grafo));
	Agraph_t *G = agread(input, NULL);

  //direcionado
  g->direcionado = agisdirected(G);

  g->vertices = povoa_vertices(g,G);
  g->arestas = povoa_arestas(G);

  //ponderado
  aresta a = conteudo(primeiro_no(g->arestas));
  if (a->peso == NULL){
    g->ponderado = 0;
  } else {
    g->ponderado = 1;
  }

  //nome
  g->nome = agnameof(G);
	return g;
}

int destroi_grafo(void *g) {
  grafo h;
  h = g;
  if (h){ return 4; }
  return 3;

}

grafo escreve_grafo(FILE *output, grafo g){
  unsigned int i;
  int ok = 1;
  no n;
  ok &= fprintf(output, "strict %sgraph \"%s\" {\n\n",
         g->direcionado ? "di" : "",
         g->nome
       );
  if (n_vertices(g)) {
    n = primeiro_no(g->vertices);
    vertice v = (vertice)conteudo(n);
    ok &= fprintf(output, "    \"%s\"\n", v->nome);
    for (i = 0; i < (n_vertices(g) - 1); ++i){
      n = proximo_no(n);
      v = (vertice)conteudo(n);
      ok &= fprintf(output, "    \"%s\"\n", v->nome);
    }
  }

  if (n_arestas(g)){
    char rep_aresta = g->direcionado ? '>' : '-';
    n = primeiro_no(g->arestas);
    aresta a = (aresta)conteudo(n);
    ok &= fprintf(output, "    \"%s\" -%c \"%s\"", a->tail, rep_aresta, a->head);

    if (ponderado(g)){
      if (a->peso){
        ok &= fprintf(output, " [peso=%s]", a->peso);
      } else {
        ok &= fprintf(output, " [peso=0]");
      }
    }

    ok &= fprintf(output, "\n");

    for (i = 0; i < (n_arestas(g) - 1); ++i){
      n = proximo_no(n);
      a = (aresta)conteudo(n);
      ok &= fprintf(output, "    \"%s\" -%c \"%s\"", a->tail, rep_aresta, a->head);

      if (ponderado(g)){
        if (a->peso){
          ok &= fprintf(output, " [peso=%s]", a->peso);
        } else {
          ok &= fprintf(output, " [peso=0]");
        }
      }

      ok &= fprintf(output, "\n");
    }
  }
  ok &= fprintf(output, "}\n");

  if (ok){
    return g;
  }else{
    return NULL;
  }
}

grafo copia_grafo(grafo g){
  grafo h;
  h = g;
  return h;
}

lista vizinhanca(vertice v, int direcao, grafo g){
  if (g->direcionado){
    if (direcao == 1){
      return v->vizinhanca[1];
    } else {
      return v->vizinhanca[0];
    }
  }
  return v->vizinhanca[0];
}

unsigned int grau(vertice v, int direcao, grafo g){
  if (g->direcionado){
    if (direcao == 1){
      return v->grau[1];
    } else {
      return v->grau[0];
    }
  }
  return v->grau[0];
}

int clique(lista l, grafo g){
  lista k = l;
  grafo h;
  h = g;
  if (h){ return 4; }
  if (k){ return 5; }
  return 3;
}

int simplicial(vertice v, grafo g){
  vertice w = v;
  grafo h;
  h = g;
  if (h){ return 4; }
  if (w){ return 5; }
  return 3;

}

int cordal(grafo g){
  grafo h;
  h = g;
  if (h){ return 4; }
  return 3;

}
