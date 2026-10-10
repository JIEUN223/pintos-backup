#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "../src/lib/kernel/rbtree.h"
struct item { int key, serial; struct rb_node node; };
static bool less(const struct rb_node *a,const struct rb_node *b,void *aux) {
 (void)aux; const struct item *x=rb_entry(a,struct item,node),*y=rb_entry(b,struct item,node);
 return x->key!=y->key?x->key<y->key:x->serial<y->serial;
}
int main(void) {
 struct item items[256]; int order[256];
 for(int seed=1;seed<=100;seed++) {
  srand(seed); struct rb_tree t; rb_init(&t,less,NULL);
  for(int i=0;i<256;i++) { items[i].key=rand()%16; items[i].serial=i; order[i]=i; rb_insert(&t,&items[i].node); assert(rb_validate(&t)); }
  struct rb_node *prev=NULL;
  for(struct rb_node *n=rb_first(&t);n;n=rb_next(&t,n)) { if(prev) assert(less(prev,n,NULL)); prev=n; }
  for(int i=255;i>0;i--) { int j=rand()%(i+1),v=order[i]; order[i]=order[j]; order[j]=v; }
  for(int i=0;i<256;i++) { rb_remove(&t,&items[order[i]].node); assert(rb_validate(&t));
   struct rb_node *previous=NULL; unsigned count=0;
   for(struct rb_node *n=rb_first(&t);n;n=rb_next(&t,n)) { if(previous) assert(less(previous,n,NULL)); previous=n; count++; }
   assert(count==255u-(unsigned)i); }
  assert(rb_first(&t)==NULL);
 }
 puts("PASS: 100 seeds, 25,600 inserts + 25,600 random removals; invariants and FIFO ties");
}
