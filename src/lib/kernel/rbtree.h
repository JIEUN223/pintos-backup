#ifndef LIB_KERNEL_RBTREE_H
#define LIB_KERNEL_RBTREE_H
#include <stdbool.h>
#include <stddef.h>
struct rb_node { struct rb_node *left, *right, *parent; bool red; };
typedef bool rb_less_func(const struct rb_node *, const struct rb_node *, void *);
struct rb_tree { struct rb_node *root; struct rb_node nil; rb_less_func *less; void *aux; size_t size; };
#define rb_entry(N,T,M) ((T *) ((char *)(N) - offsetof(T,M)))
void rb_init(struct rb_tree *, rb_less_func *, void *);
void rb_insert(struct rb_tree *, struct rb_node *);
void rb_remove(struct rb_tree *, struct rb_node *);
struct rb_node *rb_first(struct rb_tree *);
struct rb_node *rb_next(struct rb_tree *, struct rb_node *);
bool rb_validate(struct rb_tree *);
#endif
