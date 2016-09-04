#include <graphviz/cgraph.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "grafo.h"

typedef struct no *no;
typedef struct lista *lista;

struct no {

  void *conteudo;
  no proximo;
};

struct lista {

  unsigned int tamanho;
  int padding; // só pra evitar warning
  no primeiro;
};

unsigned int tamanho_lista(lista l);
no primeiro_no(lista l);
no proximo_no(no n);
void *conteudo(no n);
no insere_lista(void *conteudo, lista l);
lista constroi_lista(void);
int destroi_lista(lista l, int destroi(void *));
int remove_no(struct lista *l, struct no *rno, int destroi(void *));
lista vizinhanca(vertice v, int direcao, grafo g);

lista povoa_v_arestas(Agnode_t *v, Agraph_t *G);
lista povoa_vertices(grafo g, Agraph_t *G);
lista povoa_arestas(Agraph_t *G);
int destroi_aresta(void *a);
int destroi_vertice(void *v);
void povoa_vizinhancas(grafo g, Agraph_t *G);
int busca_vizinhanca(lista vz, vertice v);

unsigned int tamanho_lista(lista l) { return l->tamanho; }

no primeiro_no(lista l) { return l->primeiro; }

no proximo_no(no n) { return n->proximo; }

void *conteudo(no n) { return n->conteudo; }

lista constroi_lista(void) {

  lista l = malloc(sizeof(struct lista));

  if ( ! l )
    return NULL;

  l->primeiro = NULL;
  l->tamanho = 0;

  return l;
}

int destroi_lista(lista l, int destroi(void *)) {

  no p;
  int ok=1;

  while ( (p = primeiro_no(l)) ) {

    l->primeiro = proximo_no(p);

    if ( destroi )
      ok &= destroi(conteudo(p));

    free(p);
  }

  free(l);

  return ok;
}

no insere_lista(void *conteudo, lista l) {

  no novo = malloc(sizeof(struct no));

  if ( ! novo )
    return NULL;

  novo->conteudo = conteudo;
  novo->proximo = primeiro_no(l);
  ++l->tamanho;

  return l->primeiro = novo;
}

int remove_no(struct lista *l, struct no *rno, int destroi(void *)) {
	int r = 1;
	if (l->primeiro == rno) {
		l->primeiro = rno->proximo;
		if (destroi != NULL) {
			r = destroi(conteudo(rno));
		}
		free(rno);
		l->tamanho--;
		return r;
	}
	for (no n = primeiro_no(l); n->proximo; n = proximo_no(n)) {
		if (n->proximo == rno) {
			n->proximo = rno->proximo;
			if (destroi != NULL) {
				r = destroi(conteudo(rno));
			}
			free(rno);
			l->tamanho--;
			return r;
		}
	}
	return 0;
}

struct grafo {
  char *nome;
  int ponderado;
  int direcionado;


  lista vertices;
  lista arestas;
};

char *nome_grafo(grafo g){
  return g->nome;
}

int direcionado(grafo g){
  return g->direcionado;
}

int ponderado(grafo g){
  return g->ponderado;
}

unsigned int numero_vertices(grafo g){
  return tamanho_lista(g->vertices);
}

unsigned int numero_arestas(grafo g){
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

vertice vertice_nome(char *s, grafo g) {
  no n = primeiro_no(g->vertices);
  for (unsigned int i = 0; i < tamanho_lista(g->vertices); ++i){
    vertice v = conteudo(n);
    if (strcmp(v->nome,s) == 0)
      return v;
    n = proximo_no(n);
  }
  return NULL;
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
    v = vertice_nome(agnameof(n), g);

    if (g->direcionado) {
      v->vizinhanca_in = constroi_lista();
      v->vizinhanca_out = constroi_lista();
      for (e = agfstin(G,n); e; e = agnxtin(G,e)) {
        w = vertice_nome(agnameof(e->node), g);
        if (!busca_vizinhanca(v->vizinhanca_in, w))
         insere_lista(w,v->vizinhanca_in);
      }
      for (e = agfstout(G,n); e; e = agnxtout(G,e)) {
        w = vertice_nome(agnameof(e->node), g);
        if (!busca_vizinhanca(v->vizinhanca_out, w))
          insere_lista(w,v->vizinhanca_out);
      }
      v->vizinhanca = NULL;
    } else {
      v->vizinhanca = constroi_lista();
      for (e = agfstedge(G,n); e; e = agnxtedge(G,e,n)){
        w = vertice_nome(agnameof(e->node), g);
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
  agclose(G);
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

int destroi_grafo(grafo g) {
  int ok = 1;
  ok &= destroi_lista(g->vertices, destroi_vertice);
  ok &= destroi_lista(g->arestas, destroi_aresta);
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
  if (numero_vertices(g)) {
    n = primeiro_no(g->vertices);
    vertice v = (vertice)conteudo(n);
    ok &= fprintf(output, "    \"%s\"\n", v->nome);
    for (i = 0; i < (numero_vertices(g) - 1); ++i){
      n = proximo_no(n);
      v = (vertice)conteudo(n);
      ok &= fprintf(output, "    \"%s\"\n", v->nome);
    }
  }

  ok &= fprintf(output, "\n");

  if (numero_arestas(g)){
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

    for (i = 0; i < (numero_arestas(g) - 1); ++i){
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
