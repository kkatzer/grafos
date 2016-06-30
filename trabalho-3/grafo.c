#include <graphviz/cgraph.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
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
vertice busca_vertice(grafo g, char *nome);
lista povoa_arestas(Agraph_t *G);
int destroi_aresta(void *a);
int destroi_vertice(void *v);
void povoa_vizinhancas(grafo g, Agraph_t *G);
int busca_vizinhanca(lista vz, vertice v);
no insere_antes(void *conteudo, lista l, no n);

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

  lista vizinhanca;
  lista vizinhanca_in;
  lista vizinhanca_out;
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

vertice busca_vertice(grafo g, char *nome){
  no n = primeiro_no(g->vertices);
  for (unsigned int i = 0; i < tamanho_lista(g->vertices); ++i){
    vertice v = conteudo(n);
    if (strcmp(v->nome,nome) == 0)
      return v;
    n = proximo_no(n);
  }
  return NULL;
}

lista povoa_vertices(grafo g, Agraph_t *G){
  lista vertices = constroi_lista();
  vertice v;
  Agnode_t *n;
  for (n = agfstnode(G); n; n = agnxtnode(G,n)){
    v = malloc(sizeof(struct vertice));
    v->nome = agnameof(n);
    //grau e vizinhanca
    if (g->direcionado) {
      v->grau[0] = (unsigned int)agdegree(G,n,TRUE,FALSE);
      v->grau[1] = (unsigned int)agdegree(G,n,FALSE,TRUE);
    } else {
      v->grau[0] = (unsigned int)agdegree(G,n,TRUE,TRUE);
    }
    v->arestas = povoa_v_arestas(n,G);
    insere_lista(v,vertices);
  }
  return vertices;
}

int busca_vizinhanca(lista vz, vertice v){
  no n = primeiro_no(vz);
  for (unsigned int i = 0; i < tamanho_lista(vz); ++i){
    vertice w = conteudo(n);
    if (v == w)
      return 1;
    n = proximo_no(n);
  }
  return 0;
}

void povoa_vizinhancas(grafo g, Agraph_t *G){
  vertice w, v;
  Agnode_t *n;
  Agedge_t *e;
  for (n = agfstnode(G); n; n = agnxtnode(G,n)){
    v = busca_vertice(g, agnameof(n));

    if (g->direcionado) {
      v->vizinhanca_in = constroi_lista();
      v->vizinhanca_out = constroi_lista();
      for (e = agfstin(G,n); e; e = agnxtin(G,e)) {
        w = busca_vertice(g,agnameof(e->node));
        if (!busca_vizinhanca(v->vizinhanca_in, w))
         insere_lista(w,v->vizinhanca_in);
      }
      for (e = agfstout(G,n); e; e = agnxtout(G,e)) {
        w = busca_vertice(g,agnameof(e->node));
        if (!busca_vizinhanca(v->vizinhanca_out, w))
          insere_lista(w,v->vizinhanca_out);
      }
      v->vizinhanca = NULL;
    } else {
      v->vizinhanca = constroi_lista();
      for (e = agfstedge(G,n); e; e = agnxtedge(G,e,n)){
        w = busca_vertice(g,agnameof(e->node));
        if (!busca_vizinhanca(v->vizinhanca, w))
         insere_lista(w,v->vizinhanca);
      }
      v->vizinhanca_in = v->vizinhanca_out = NULL;
    }
  }
  return;
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
  povoa_vizinhancas(g,G);

  //ponderado
  aresta a = conteudo(primeiro_no(g->arestas));
  if (a->peso == NULL){
    g->ponderado = 0;
  } else {
    g->ponderado = 1;
  }

  //nome
  g->nome = agnameof(G);
  // agclose(G);
	return g;
}

int destroi_aresta(void *a){
  free(a);
  return 1;
}

int destroi_vertice(void *v){
  int ok = 1;
  vertice w = (vertice)v;
  if (w->vizinhanca != NULL){
    ok &= destroi_lista(w->vizinhanca, NULL);
  }
  if (w->vizinhanca_in != NULL){
    ok &= destroi_lista(w->vizinhanca_in, NULL);
  }
  if (w->vizinhanca_out != NULL){
    ok &= destroi_lista(w->vizinhanca_out, NULL);
  }
  ok &= destroi_lista(w->arestas, destroi_aresta);
  free(v);
  return ok;
}

int destroi_grafo(void *g) {
  int ok = 1;
  grafo h = (grafo)g;
  ok &= destroi_lista(h->vertices, destroi_vertice);
  ok &= destroi_lista(h->arestas, destroi_aresta);
  free(g);
  return ok;
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

  ok &= fprintf(output, "\n");

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
  unsigned int i;
  no n;
  char *graph = malloc(100*sizeof(char));
  char *aux = malloc(100*sizeof(char));
  snprintf(aux, 100, "strict %sgraph \"%s\" {\n\n",
         g->direcionado ? "di" : "",
         g->nome
       );
  graph = strcat(graph, aux);
  if (n_vertices(g)) {
    n = primeiro_no(g->vertices);
    vertice v = (vertice)conteudo(n);
    snprintf(aux, 100, "    \"%s\"\n", v->nome);
    graph = strcat(graph, aux);
    for (i = 0; i < (n_vertices(g) - 1); ++i){
      n = proximo_no(n);
      v = (vertice)conteudo(n);
      snprintf(aux, 100, "    \"%s\"\n", v->nome);
      graph = strcat(graph, aux);
    }
  }

  graph = strcat(graph, "\n");

  if (n_arestas(g)){
    char rep_aresta = g->direcionado ? '>' : '-';
    n = primeiro_no(g->arestas);
    aresta a = (aresta)conteudo(n);
    snprintf(aux, 100, "    \"%s\" -%c \"%s\"", a->tail, rep_aresta, a->head);
    graph = strcat(graph, aux);

    if (ponderado(g)){
      if (a->peso){
        snprintf(aux, 100, " [peso=%s]", a->peso);
        graph = strcat(graph, aux);
      } else {
        snprintf(aux, 100, " [peso=0]");
        graph = strcat(graph, aux);
      }
    }

    graph = strcat(graph, "\n");

    for (i = 0; i < (n_arestas(g) - 1); ++i){
      n = proximo_no(n);
      a = (aresta)conteudo(n);
      snprintf(aux, 100, "    \"%s\" -%c \"%s\"", a->tail, rep_aresta, a->head);
      graph = strcat(graph, aux);

      if (ponderado(g)){
        if (a->peso){
          snprintf(aux, 100, " [peso=%s]", a->peso);
          graph = strcat(graph, aux);
        } else {
          snprintf(aux, 100, " [peso=0]");
          graph = strcat(graph, aux);
        }
      }

      graph = strcat(graph, "\n");
    }
  }
  graph = strcat(graph, "}\n");

	grafo h = malloc(sizeof(struct grafo));
	Agraph_t *G = agmemread(graph);

  //direcionado
  h->direcionado = agisdirected(G);

  h->vertices = povoa_vertices(h,G);
  h->arestas = povoa_arestas(G);
  povoa_vizinhancas(h,G);

  //ponderado
  aresta a = conteudo(primeiro_no(h->arestas));
  if (a->peso == NULL){
    h->ponderado = 0;
  } else {
    h->ponderado = 1;
  }

  //nome
  h->nome = agnameof(G);
  agclose(G);
  vertice v = conteudo(primeiro_no(h->vertices));
  lista l = vizinhanca(v,0,h);
  if (l == NULL){
    printf("alala");
  }
  // v = conteudo(primeiro_no(vizinhanca(v,0,h)));
  // printf("%s", v->nome);
  return h;
}

lista vizinhanca(vertice v, int direcao, grafo g){
  if (g->direcionado){
    if (direcao == 1){
      return v->vizinhanca_out;
    } else {
      return v->vizinhanca_in;
    }
  }
  return v->vizinhanca;
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

//------------------------------------------------------------------------------
// devolve 1, se o conjunto dos vertices em l é uma clique em g, ou
//         0, caso contrário
//
// um conjunto C de vértices de um grafo é uma clique em g
// se todo vértice em C é vizinho de todos os outros vértices de C em g

int clique(lista l, grafo g){
  if (g->direcionado)
    return 0;
  int ok = 1;
  int nok = 0;
  vertice v,w;
  no n = primeiro_no(l);
  for (unsigned int i = 0; i < tamanho_lista(l); ++i){
    v = conteudo(n);
    no m = primeiro_no(v->vizinhanca);
    nok = 0;
    for (unsigned int j = 0; j < tamanho_lista(v->vizinhanca); ++j){
      w = conteudo(m);
      nok |= (v == w);
      m = proximo_no(m);
    }
    ok &= nok;
    n = proximo_no(n);
  }
  return ok;
}

//------------------------------------------------------------------------------
// devolve 1, se v é um vértice simplicial em g, ou
//         0, caso contrário
//
// um vértice é simplicial no grafo se sua vizinhança é uma clique

int simplicial(vertice v, grafo g){
  if (g->direcionado)
    return 0;
  return clique(v->vizinhanca,g);
}

//------------------------------------------------------------------------------
// devolve uma lista de vertices com a ordem dos vértices dada por uma
// busca em largura lexicográfica

lista busca_largura_lexicografica(grafo g){
  vertice x,z;
  lista viz, Y, pi, L, S = constroi_lista();
  no m, n = primeiro_no(g->vertices);
  while (n != NULL) {
    x = conteudo(n);
    insere_lista(x,S);
    n = proximo_no(n);
  }
  L = constroi_lista();
  insere_lista(S,L);
  pi = constroi_lista();

  while (tamanho_lista(L) != 0) {
    n = primeiro_no(L);
    S = conteudo(n);
    m = primeiro_no(S);
    x = conteudo(m);
    remove_no(S,m,NULL);
    if (tamanho_lista(S) == 0) {
      remove_no(L,n,NULL);
      destroi_lista(S,NULL);
    }
    insere_lista(x,pi);

    n = primeiro_no(L);
    while (n != NULL) {
      Y = constroi_lista();
      S = conteudo(n);
      m = primeiro_no(S);
      while (m != NULL) {
        z = conteudo(m);
        viz = vizinhanca(x,0,g);
        if (busca_vizinhanca(viz,z)) {
          insere_lista(z,Y);
          remove_no(S,m,NULL);
        }
        m = proximo_no(m);
      }
      if (tamanho_lista(Y) != 0) insere_lista(Y,L);
      else destroi_lista(Y,NULL);
      if (tamanho_lista(S) == 0) {
        remove_no(L,n,NULL);
        destroi_lista(S,NULL);
      }
      n = proximo_no(n);
    }
  }
  return pi;
}

//------------------------------------------------------------------------------
// devolve 1, se a lista l representa uma
//            ordem perfeita de eliminação para o grafo g ou
//         0, caso contrário
//
// o tempo de execução é O(|V(G)|+|E(G)|)

int ordem_perfeita_eliminacao(lista l, grafo g){
  no m, n;
  vertice v, w;
  lista viz;
  n = primeiro_no(l);
  while (n != NULL) {
    v = conteudo(n);
    m = proximo_no(n);
    while (m != NULL) {
      w = conteudo(m);
      viz = vizinhanca(v,0,g);
      if (!busca_vizinhanca(viz,w)) {
        destroi_lista(l,NULL);
        return 0;
      }
      m = proximo_no(m);
    }

    remove_no(l,n,NULL);
    n = primeiro_no(l);
  }
  destroi_lista(l,NULL);
  return 1;
}

//------------------------------------------------------------------------------
// devolve 1, se g é um grafo cordal ou
//         0, caso contrário

int cordal(grafo g){
  lista l = busca_largura_lexicografica(g);
  return ordem_perfeita_eliminacao(l,g);
}
