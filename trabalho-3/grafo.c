#include <graphviz/cgraph.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include "grafo.h"

#define true 1
#define false 0

typedef struct aresta *aresta;

lista povoa_vertices(grafo g, Agraph_t *G);
vertice busca_vertice(grafo g, char *nome);
lista povoa_arestas(grafo g, Agraph_t *G);
int destroi_aresta(void *a);
int destroi_vertice(void *v);
void povoa_vizinhancas(grafo g, Agraph_t *G);
int busca_vizinhanca(lista vz, vertice v);
no insere_antes(void *conteudo, lista l, no n);
lista caminho_aumentante(grafo g);
void desvisita_vertices(grafo g);
int busca_caminho(vertice v, lista l, int last);
void xor(lista l);
grafo copia_grafo_emparelhado(grafo g);

//---------------------------------------------------------------------------
// nó de lista encadeada cujo conteúdo é um void *

struct no {

  void *conteudo;
  no proximo;
};
//---------------------------------------------------------------------------
// lista encadeada

struct lista {

  unsigned int tamanho;
  int padding; // só pra evitar warning
  no primeiro;
};
//---------------------------------------------------------------------------
// devolve o número de nós da lista l

unsigned int tamanho_lista(lista l) { return l->tamanho; }

//---------------------------------------------------------------------------
// devolve o primeiro nó da lista l,
//      ou NULL, se l é vazia

no primeiro_no(lista l) { return l->primeiro; }

//---------------------------------------------------------------------------
// devolve o conteúdo do nó n
//      ou NULL se n = NULL

void *conteudo(no n) { return n->conteudo; }

//---------------------------------------------------------------------------
// devolve o sucessor do nó n,
//      ou NULL, se n for o último nó da lista

no proximo_no(no n) { return n->proximo; }

//---------------------------------------------------------------------------
// cria uma lista vazia e a devolve
//
// devolve NULL em caso de falha

lista constroi_lista(void) {

  lista l = malloc(sizeof(struct lista));

  if ( ! l )
    return NULL;

  l->primeiro = NULL;
  l->tamanho = 0;

  return l;
}
//---------------------------------------------------------------------------
// desaloca a lista l e todos os seus nós
//
// se destroi != NULL invoca
//
//     destroi(conteudo(n))
//
// para cada nó n da lista.
//
// devolve 1 em caso de sucesso,
//      ou 0 em caso de falha

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
//---------------------------------------------------------------------------
// insere um novo nó na lista l cujo conteúdo é p
//
// devolve o no recém-criado
//      ou NULL em caso de falha

no insere_lista(void *conteudo, lista l) {

  no novo = malloc(sizeof(struct no));

  if ( ! novo )
    return NULL;

  novo->conteudo = conteudo;
  novo->proximo = primeiro_no(l);
  ++l->tamanho;

  return l->primeiro = novo;
}

//------------------------------------------------------------------------------
// remove o no de endereço rno de l
// se destroi != NULL, executa destroi(conteudo(rno))
// devolve 1, em caso de sucesso
//         0, se rno não for um no de l

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

unsigned int n_vertices(grafo g){
  return tamanho_lista(g->vertices);
}

unsigned int n_arestas(grafo g){
  return tamanho_lista(g->arestas);
}

struct vertice {
  char *nome;
  unsigned int grau[2];
  int visitado;
  int coberto;

  lista vizinhanca;
  lista vizinhanca_in;
  lista vizinhanca_out;
  lista arestas;
};

char *nome_vertice(vertice v){
  return v->nome;
}

struct aresta {
  char *nome;
  char *peso;
  int coberta;
  int padding; // só pra evitar warning

  vertice head;
  vertice tail;
};

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
    v->visitado = 0;
    v->coberto = 0;
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

lista povoa_arestas(grafo g, Agraph_t *G){
  aresta a = NULL;
  vertice v = NULL;
  char peso[] = "peso";
  lista arestas = constroi_lista();
  for (Agnode_t *n = agfstnode(G); n; n = agnxtnode(G,n)) {
    for (Agedge_t *e = agfstout(G,n); e; e = agnxtout(G,e)) {
      a = malloc(sizeof(struct aresta));
      a->head = busca_vertice(g, agnameof(aghead(e)));
      a->tail = busca_vertice(g, agnameof(agtail(e)));
      a->nome = agnameof(e);
      a->peso = agget(e,peso);
      a->coberta = 0;
      if (insere_lista(a,arestas) == NULL){
        return NULL;
      }
      v = a->head;
      if (v->arestas == NULL)
        v->arestas = constroi_lista();
      insere_lista(a,v->arestas);
      v = a->tail;
      if (v->arestas == NULL)
        v->arestas = constroi_lista();
      insere_lista(a,v->arestas);
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
  g->arestas = povoa_arestas(g,G);
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
    ok &= fprintf(output, "    \"%s\" -%c \"%s\"", a->tail->nome, rep_aresta, a->head->nome);

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
      ok &= fprintf(output, "    \"%s\" -%c \"%s\"", a->tail->nome, rep_aresta, a->head->nome);

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
	grafo h = malloc(sizeof(struct grafo));

  //direcionado
  h->direcionado = g->directionado;

  h->vertices = copia_vertices(g);
  h->arestas = copia_arestas(g,h);
  copia_vizinhancas(h);

  //ponderado
  h->ponderado = g->ponderado

  //nome
  h->nome = g->nome;
  return h;
}

lista copia_vertices(grafo g) {
  lista l = constroi_lista();
  no n;
  vertice v, w;
  for (n = primeiro_no(g->vertices); n != NULL; n = proximo_no(n)) {
    v = conteudo(n);
    w = malloc(sizeof(struct vertice));
    w->nome = malloc(sizeof(g->nome));
    strcpy(w->nome,g->nome);
    if (g->direcionado) {
      w->grau[0] = v->grau[0];
      w->grau[1] = v->grau[1];
    } else {
      w->grau[0] = v->grau[0];
    }
    insere_lista(w,l);
  }
  return l;
}

lista copia_arestas(grafo g, grafo h) {
  lista l = constroi_lista();
  no n;
  aresta a, b;
  for (n = primeiro_no(g->arestas); n != NULL; n = proximo_no(n)) {
    a = conteudo(n);
    b = malloc(sizeof(struct aresta));
    b->nome = malloc(sizeof(a->nome));
    strcpy(b->nome,a->nome);
    b->peso = malloc(sizeof(a->peso));
    strcpy(b->peso,a->peso);

    b->head = busca_vertice(h,a->head->nome);
    b->tail = busca_vertice(h,a->tail->nome);
  }
}

void copia_vizinhancas(grafo g) {
  lista l = constroi_lista();
  //TODO
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

//------------------------------------------------------------------------------
// devolve um grafo cujos vertices são cópias de vértices do grafo
// bipartido g e cujas arestas formam um emparelhamento máximo em g
//
// o grafo devolvido, portanto, é vazio ou tem todos os vértices com grau 1
//
// não verifica se g é bipartido; caso não seja, o comportamento é indefinido

grafo emparelhamento_maximo(grafo g) {
  lista l = caminho_aumentante(g);
   while (l) {
     xor(l);
     desvisita_vertices(g);
     l = caminho_aumentante(g);
   }
   grafo e = copia_grafo(g);
   return g;
}

lista caminho_aumentante(grafo g) {
  lista l;
  no n;
  vertice v;
  for (n = primeiro_no(g->vertices); n != NULL; n = proximo_no(n)) {
    v = conteudo(n);
    if (v->coberto == false){
      v->visitado = true;
      l = constroi_lista();
      if (busca_caminho(v,l,1)) {
        desvisita_vertices(g);
        return l;
      }
      desvisita_vertices(g);
      destroi_lista(l,NULL);
    }
  }
  return NULL;
}

void desvisita_vertices(grafo g) {
  no n;
  vertice v;
  for (n = primeiro_no(g->vertices); n != NULL; n = proximo_no(n)) {
    v = conteudo(n);
    v->visitado = false;
  }
  return;
}

int busca_caminho(vertice v, lista l, int last) {
  no n;
  aresta a;
  vertice vizinho;
  if (!v->coberto && !v->visitado)
  return true;
  v->visitado = true;
  for (n = primeiro_no(v->arestas); n != NULL; n = proximo_no(n)) {
    a = conteudo(n);
    if (a->coberta != last) {
      if (a->tail == v) vizinho = a->head;
      else vizinho = a->tail;
      if (!vizinho->visitado && busca_caminho(vizinho, l, !last)) {
        insere_lista(a, l);
        return true;
      }
    }
  }
  return false;
}

void xor(lista l) {
  no n;
  aresta a;
  vertice v;
  for (n = primeiro_no(l); n != NULL; n = proximo_no(n)) {
    a = conteudo(n);
    a->coberta = !a->coberta;
    if (a->coberta) {
      v = a->head;
      v->coberto = true;
      v = a->tail;
      v->coberto = true;
    } else {
      if (v->visitado) {
        v = a->head;
        v->coberto = false;
        v = a->tail;
        v->coberto = false;
      }
      v->visitado = true;
    }
  }
}