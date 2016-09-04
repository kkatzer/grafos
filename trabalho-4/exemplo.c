#include <stdio.h>
#include <stdlib.h>
#include <graphviz/cgraph.h>

//------------------------------------------------------------------------------
static void mostra_arestas(Agraph_t *g) {

  char rep_aresta = agisdirected(g) ? '>' : '-';
  
  for (Agnode_t *v=agfstnode(g); v; v=agnxtnode(g,v))

    for (Agedge_t *a=agfstedge(g,v); a; a=agnxtedge(g,a,v)) {
  
      if (v == agtail(a)) {

        char *peso = agget(a, (char *)"peso");
	  
        printf("    \"%s\" -%c \"%s\"",
               agnameof(agtail(a)),
               rep_aresta,
               agnameof(aghead(a))
               );

        if ( peso && *peso )
          printf(" [peso=%s]", peso);

        printf("\n");
      }
    }
}
//------------------------------------------------------------------------------
static void mostra_vertices(Agraph_t *g) {

  for (Agnode_t *v=agfstnode(g); v; v=agnxtnode(g,v))

    printf("    \"%s\"\n", agnameof(v));
}
//------------------------------------------------------------------------------
static Agraph_t *mostra_grafo(Agraph_t *g) {

  if ( !g )
    return NULL;

  printf("strict %sgraph \"%s\" {\n\n",
         agisdirected(g) ? "di" : "",
         agnameof(g)
         );

  mostra_vertices(g);

  printf("\n");

  mostra_arestas(g);

  printf("}\n");

  return g;
}
//------------------------------------------------------------------------------
int main(void) {

  Agraph_t *g = agread(stdin, NULL);

  if ( !g )
    return 1;

  agclose(mostra_grafo(g));

  return agerrors();
}
