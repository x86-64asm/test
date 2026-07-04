#include "musab_lang.h"

// Type system for Musab Language

static Type *ty_int = NULL;
static Type *ty_float = NULL;
static Type *ty_string = NULL;
static Type *ty_bool = NULL;
static Type *ty_void = NULL;

Type *new_type(TypeKind kind, int size, int align) {
  Type *ty = calloc(1, sizeof(Type));
  ty->kind = kind;
  ty->size = size;
  ty->align = align;
  return ty;
}

Type *get_int_type(void) {
  if (!ty_int) {
    ty_int = new_type(TY_INT, 8, 8);  // 64-bit int
  }
  return ty_int;
}

Type *get_float_type(void) {
  if (!ty_float) {
    ty_float = new_type(TY_FLOAT, 8, 8);  // 64-bit double
  }
  return ty_float;
}

Type *get_string_type(void) {
  if (!ty_string) {
    ty_string = new_type(TY_STRING, 8, 8);  // Pointer to string
  }
  return ty_string;
}

Type *get_bool_type(void) {
  if (!ty_bool) {
    ty_bool = new_type(TY_BOOL, 1, 1);
  }
  return ty_bool;
}

Type *get_void_type(void) {
  if (!ty_void) {
    ty_void = new_type(TY_VOID, 0, 1);
  }
  return ty_void;
}

Type *pointer_to(Type *base) {
  Type *ty = new_type(TY_PTR, 8, 8);
  ty->base = base;
  return ty;
}

Type *array_of(Type *base, int len) {
  Type *ty = new_type(TY_ARRAY, base->size * len, base->align);
  ty->base = base;
  ty->array_len = len;
  return ty;
}

Type *func_type(Type *return_ty) {
  Type *ty = new_type(TY_FUNC, 8, 8);
  ty->return_ty = return_ty;
  return ty;
}

bool is_integer_type(Type *ty) {
  return ty && (ty->kind == TY_INT || ty->kind == TY_BOOL);
}

bool is_float_type(Type *ty) {
  return ty && ty->kind == TY_FLOAT;
}

bool is_numeric_type(Type *ty) {
  return is_integer_type(ty) || is_float_type(ty);
}

bool is_string_type(Type *ty) {
  return ty && ty->kind == TY_STRING;
}

bool is_compatible(Type *t1, Type *t2) {
  if (!t1 || !t2) return false;
  return t1->kind == t2->kind;
}

void add_type(Node *node) {
  if (!node || node->ty) return;
  
  // Recursively add types to child nodes
  if (node->lhs) add_type(node->lhs);
  if (node->rhs) add_type(node->rhs);
  if (node->cond) add_type(node->cond);
  if (node->then) add_type(node->then);
  if (node->els) add_type(node->els);
  
  switch (node->kind) {
    case ND_NUM:
      node->ty = get_int_type();
      break;
    case ND_STR:
      node->ty = get_string_type();
      break;
    case ND_ADD:
    case ND_SUB:
    case ND_MUL:
    case ND_DIV:
    case ND_MOD:
      if (node->lhs->ty->kind == TY_FLOAT || node->rhs->ty->kind == TY_FLOAT) {
        node->ty = get_float_type();
      } else {
        node->ty = get_int_type();
      }
      break;
    case ND_EQ:
    case ND_NE:
    case ND_LT:
    case ND_LE:
    case ND_GT:
    case ND_GE:
    case ND_LOGAND:
    case ND_LOGOR:
    case ND_NOT:
      node->ty = get_bool_type();
      break;
    case ND_ASSIGN:
      node->ty = node->lhs->ty;
      break;
    case ND_VAR:
      // Type should already be set from symbol table
      break;
    default:
      node->ty = get_void_type();
      break;
  }
}
