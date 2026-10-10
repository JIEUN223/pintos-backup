#include "rbtree.h"
static void left_rotate(struct rb_tree *t, struct rb_node *x) {
 struct rb_node *y=x->right; x->right=y->left;
 if(y->left!=&t->nil) y->left->parent=x;
 y->parent=x->parent;
 if(x->parent==&t->nil) t->root=y;
 else if(x==x->parent->left) x->parent->left=y;
 else x->parent->right=y;
 y->left=x; x->parent=y;
}
static void right_rotate(struct rb_tree *t, struct rb_node *x) {
 struct rb_node *y=x->left; x->left=y->right;
 if(y->right!=&t->nil) y->right->parent=x;
 y->parent=x->parent;
 if(x->parent==&t->nil) t->root=y;
 else if(x==x->parent->right) x->parent->right=y;
 else x->parent->left=y;
 y->right=x; x->parent=y;
}
void rb_init(struct rb_tree *t, rb_less_func *less, void *aux) {
 t->nil.left=t->nil.right=t->nil.parent=&t->nil;
 t->nil.red=false; t->root=&t->nil; t->less=less; t->aux=aux; t->size=0;
}
void rb_insert(struct rb_tree *t, struct rb_node *z) {
 struct rb_node *p=&t->nil, *x=t->root;
 while(x!=&t->nil) { p=x; x=t->less(z,x,t->aux)?x->left:x->right; }
 z->parent=p; z->left=z->right=&t->nil; z->red=true;
 if(p==&t->nil) t->root=z;
 else if(t->less(z,p,t->aux)) p->left=z;
 else p->right=z;
 t->size++;
 while(z->parent->red) {
  struct rb_node *g=z->parent->parent;
  if(z->parent==g->left) {
   struct rb_node *u=g->right;
   if(u->red) { z->parent->red=false; u->red=false; g->red=true; z=g; }
   else {
    if(z==z->parent->right) { z=z->parent; left_rotate(t,z); }
    z->parent->red=false; z->parent->parent->red=true; right_rotate(t,z->parent->parent);
   }
  } else {
   struct rb_node *u=g->left;
   if(u->red) { z->parent->red=false; u->red=false; g->red=true; z=g; }
   else {
    if(z==z->parent->left) { z=z->parent; right_rotate(t,z); }
    z->parent->red=false; z->parent->parent->red=true; left_rotate(t,z->parent->parent);
   }
  }
 }
 t->root->red=false;
}
static struct rb_node *minimum(struct rb_tree *t, struct rb_node *x) {
 while(x->left!=&t->nil) x=x->left;
 return x;
}
struct rb_node *rb_first(struct rb_tree *t) { return t->root==&t->nil?NULL:minimum(t,t->root); }
struct rb_node *rb_next(struct rb_tree *t, struct rb_node *x) {
 if(x->right!=&t->nil) return minimum(t,x->right);
 struct rb_node *p=x->parent;
 while(p!=&t->nil && x==p->right) { x=p; p=p->parent; }
 return p==&t->nil?NULL:p;
}
static void transplant(struct rb_tree *t, struct rb_node *u, struct rb_node *v) {
 if(u->parent==&t->nil) t->root=v;
 else if(u==u->parent->left) u->parent->left=v;
 else u->parent->right=v;
 v->parent=u->parent;
}
void rb_remove(struct rb_tree *t, struct rb_node *z) {
 struct rb_node *y=z, *x;
 bool red=y->red;
 if(z->left==&t->nil) { x=z->right; transplant(t,z,z->right); }
 else if(z->right==&t->nil) { x=z->left; transplant(t,z,z->left); }
 else {
  y=minimum(t,z->right); red=y->red; x=y->right;
  if(y->parent==z) x->parent=y;
  else { transplant(t,y,y->right); y->right=z->right; y->right->parent=y; }
  transplant(t,z,y); y->left=z->left; y->left->parent=y; y->red=z->red;
 }
 t->size--;
 if(!red) {
  while(x!=t->root && !x->red) {
   if(x==x->parent->left) {
    struct rb_node *w=x->parent->right;
    if(w->red) { w->red=false; x->parent->red=true; left_rotate(t,x->parent); w=x->parent->right; }
    if(!w->left->red && !w->right->red) { w->red=true; x=x->parent; }
    else {
     if(!w->right->red) { w->left->red=false; w->red=true; right_rotate(t,w); w=x->parent->right; }
     w->red=x->parent->red; x->parent->red=false; w->right->red=false; left_rotate(t,x->parent); x=t->root;
    }
   } else {
    struct rb_node *w=x->parent->left;
    if(w->red) { w->red=false; x->parent->red=true; right_rotate(t,x->parent); w=x->parent->left; }
    if(!w->right->red && !w->left->red) { w->red=true; x=x->parent; }
    else {
     if(!w->left->red) { w->right->red=false; w->red=true; left_rotate(t,w); w=x->parent->left; }
     w->red=x->parent->red; x->parent->red=false; w->left->red=false; right_rotate(t,x->parent); x=t->root;
    }
   }
  }
  x->red=false;
 }
 t->nil.red=false;
}
static int validate_node(struct rb_tree *t, struct rb_node *n, size_t *count) {
 if(n==&t->nil) return 1;
 (*count)++;
 if(n->left!=&t->nil && (n->left->parent!=n || !t->less(n->left,n,t->aux))) return -1;
 if(n->right!=&t->nil && (n->right->parent!=n || t->less(n->right,n,t->aux))) return -1;
 if(n->red && (n->left->red || n->right->red)) return -1;
 int l=validate_node(t,n->left,count), r=validate_node(t,n->right,count);
 return l<0 || l!=r ? -1 : l+!n->red;
}
bool rb_validate(struct rb_tree *t) {
 size_t count=0;
 if(t->root->red || (t->root!=&t->nil && t->root->parent!=&t->nil)) return false;
 return validate_node(t,t->root,&count)>0 && count==t->size;
}
