#include "lottery_rbt.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>

static int get_subtree_total(struct ticket_node *node) {
  return node ? node->subtree_total : 0;
}

static void update_subtree_total(struct ticket_node *node) {
  if (node)
    node->subtree_total = node->tickets + get_subtree_total(node->left) + get_subtree_total(node->right);
}

int rbt_total_tickets(struct ticket_node *root) {
  return get_subtree_total(root);
}

struct thread *rbt_pick(struct ticket_node *node, int ticket) {
  while (node) {
    int left_total = get_subtree_total(node->left);
    if (ticket <= left_total) {
      node = node->left;
    } else if (ticket <= left_total + node->tickets) {
      return node->t;
    } else {
      ticket -= (left_total + node->tickets);
      node = node->right;
    }
  }
  return NULL;
}

static void left_rotate(struct ticket_node **root, struct ticket_node *x) {
  struct ticket_node *y = x->right;
  x->right = y->left;
  if (y->left) y->left->parent = x;
  y->parent = x->parent;
  if (!x->parent) *root = y;
  else if (x == x->parent->left) x->parent->left = y;
  else x->parent->right = y;
  y->left = x;
  x->parent = y;

  update_subtree_total(x);
  update_subtree_total(y);
}

static void right_rotate(struct ticket_node **root, struct ticket_node *y) {
  struct ticket_node *x = y->left;
  y->left = x->right;
  if (x->right) x->right->parent = y;
  x->parent = y->parent;
  if (!y->parent) *root = x;
  else if (y == y->parent->left) y->parent->left = x;
  else y->parent->right = x;
  x->right = y;
  y->parent = x;

  update_subtree_total(y);
  update_subtree_total(x);
}

static void insert_fixup(struct ticket_node **root, struct ticket_node *z) {
  while (z->parent && z->parent->color == RED) {
    if (z->parent == z->parent->parent->left) {
      struct ticket_node *y = z->parent->parent->right;
      if (y && y->color == RED) {
        z->parent->color = BLACK;
        y->color = BLACK;
        z->parent->parent->color = RED;
        z = z->parent->parent;
      } else {
        if (z == z->parent->right) {
          z = z->parent;
          left_rotate(root, z);
        }
        z->parent->color = BLACK;
        z->parent->parent->color = RED;
        right_rotate(root, z->parent->parent);
      }
    } else {
      struct ticket_node *y = z->parent->parent->left;
      if (y && y->color == RED) {
        z->parent->color = BLACK;
        y->color = BLACK;
        z->parent->parent->color = RED;
        z = z->parent->parent;
      } else {
        if (z == z->parent->left) {
          z = z->parent;
          right_rotate(root, z);
        }
        z->parent->color = BLACK;
        z->parent->parent->color = RED;
        left_rotate(root, z->parent->parent);
      }
    }
  }
  (*root)->color = BLACK;
}

void rbt_insert(struct ticket_node **root, struct thread *t) {
  struct ticket_node *z = malloc(sizeof(struct ticket_node));
  z->t = t;
  z->tickets = t->tickets;
  z->subtree_total = t->tickets;
  z->left = z->right = z->parent = NULL;
  z->color = RED;

  struct ticket_node *y = NULL;
  struct ticket_node *x = *root;

  while (x != NULL) {
    y = x;
    x->subtree_total += z->tickets;
    if (t->tid < x->t->tid)
      x = x->left;
    else
      x = x->right;
  }

  z->parent = y;
  if (!y) *root = z;
  else if (t->tid < y->t->tid) y->left = z;
  else y->right = z;

  insert_fixup(root, z);
}

static void transplant(struct ticket_node **root, struct ticket_node *u, struct ticket_node *v) {
  if (!u->parent) *root = v;
  else if (u == u->parent->left) u->parent->left = v;
  else u->parent->right = v;
  if (v) v->parent = u->parent;
}

static void remove_fixup(struct ticket_node **root, struct ticket_node *x, struct ticket_node *x_parent) {
  while (x != *root && (!x || x->color == BLACK)) {
    if (x == x_parent->left) {
      struct ticket_node *w = x_parent->right;
      if (w && w->color == RED) {
        w->color = BLACK;
        x_parent->color = RED;
        left_rotate(root, x_parent);
        w = x_parent->right;
      }
      if ((!w->left || w->left->color == BLACK) && (!w->right || w->right->color == BLACK)) {
        w->color = RED;
        x = x_parent;
        x_parent = x->parent;
      } else {
        if (!w->right || w->right->color == BLACK) {
          if (w->left) w->left->color = BLACK;
          w->color = RED;
          right_rotate(root, w);
          w = x_parent->right;
        }
        w->color = x_parent->color;
        x_parent->color = BLACK;
        if (w->right) w->right->color = BLACK;
        left_rotate(root, x_parent);
        x = *root;
        break;
      }
    } else {
      struct ticket_node *w = x_parent->left;
      if (w && w->color == RED) {
        w->color = BLACK;
        x_parent->color = RED;
        right_rotate(root, x_parent);
        w = x_parent->left;
      }
      if ((!w->right || w->right->color == BLACK) && (!w->left || w->left->color == BLACK)) {
        w->color = RED;
        x = x_parent;
        x_parent = x->parent;
      } else {
        if (!w->left || w->left->color == BLACK) {
          if (w->right) w->right->color = BLACK;
          w->color = RED;
          left_rotate(root, w);
          w = x_parent->left;
        }
        w->color = x_parent->color;
        x_parent->color = BLACK;
        if (w->left) w->left->color = BLACK;
        right_rotate(root, x_parent);
        x = *root;
        break;
      }
    }
  }
  if (x) x->color = BLACK;
}

void rbt_remove(struct ticket_node **root, struct thread *t) {
  struct ticket_node *z = *root;
  while (z && z->t != t) {
    if (t->tid < z->t->tid) z = z->left;
    else z = z->right;
  }
  if (!z) return;

  struct ticket_node *p = z->parent;
  while (p) {
    p->subtree_total -= z->tickets;
    p = p->parent;
  }

  struct ticket_node *y = z;
  struct ticket_node *x = NULL;
  struct ticket_node *x_parent = NULL;
  enum color y_original_color = y->color;

  if (!z->left) {
    x = z->right;
    x_parent = z->parent;
    transplant(root, z, z->right);
  } else if (!z->right) {
    x = z->left;
    x_parent = z->parent;
    transplant(root, z, z->left);
  } else {
    y = z->right;
    while (y->left) y = y->left;
    y_original_color = y->color;
    x = y->right;
    if (y->parent == z) {
      if (x) x->parent = y;
      x_parent = y;
    } else {
      transplant(root, y, y->right);
      y->right = z->right;
      if (y->right) y->right->parent = y;
      x_parent = y->parent;
    }
    transplant(root, z, y);
    y->left = z->left;
    if (y->left) y->left->parent = y;
    y->color = z->color;
    update_subtree_total(y);
  }

  free(z);
  if (y_original_color == BLACK)
    remove_fixup(root, x, x_parent);
}
