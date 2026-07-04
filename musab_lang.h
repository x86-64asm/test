// Musab Language Compiler Header
// Customized from chibicc for Musab's language syntax

#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <glob.h>
#include <libgen.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define MAX(x, y) ((x) < (y) ? (y) : (x))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

#ifndef __GNUC__
# define __attribute__(x)
#endif

typedef struct Type Type;
typedef struct Node Node;
typedef struct Member Member;
typedef struct Token Token;

// Token types for Musab Language
typedef enum {
  TK_IDENT,      // Identifiers
  TK_PUNCT,      // Punctuators
  TK_KEYWORD,    // Keywords (var, const, fn, struct, enum, etc.)
  TK_STR,        // String literals
  TK_NUM,        // Numeric literals (int/float)
  TK_EOF,        // End-of-file
} TokenKind;

typedef struct {
  char *name;
  int file_no;
  char *contents;
} File;

struct Token {
  TokenKind kind;
  Token *next;
  int64_t val;            // For TK_NUM
  long double fval;       // For float/double literals
  char *loc;
  int len;
  Type *ty;
  char *str;              // String literal contents
  File *file;
  char *filename;
  int line_no;
  bool at_bol;
  bool has_space;
};

// Keywords for Musab Language
typedef enum {
  KW_VAR,        // var keyword
  KW_CONST,      // const keyword
  KW_FN,         // fn keyword
  KW_STRUCT,     // struct keyword
  KW_ENUM,       // enum keyword
  KW_USE,        // use keyword
  KW_IF,         // if keyword
  KW_ELSE,       // else keyword
  KW_SWITCH,     // switch keyword
  KW_CASE,       // case keyword
  KW_DEFAULT,    // default keyword
  KW_WHILE,      // while keyword
  KW_FOR,        // for keyword
  KW_BREAK,      // break keyword
  KW_CONTINUE,   // continue keyword
  KW_RETURN,     // return keyword
  KW_NONE,       // not a keyword
} KeywordType;

// Type kinds for Musab Language
typedef enum {
  TY_INT,
  TY_FLOAT,
  TY_STRING,
  TY_BOOL,
  TY_VOID,
  TY_ARRAY,
  TY_STRUCT,
  TY_ENUM,
  TY_PTR,
  TY_FUNC,
} TypeKind;

struct Type {
  TypeKind kind;
  int size;
  int align;
  Type *base;           // For arrays/pointers
  int array_len;        // For arrays
  Type *return_ty;      // For functions
  Type *params;         // For functions
  Member *members;      // For structs
  bool is_const;
};

// Struct member
struct Member {
  Member *next;
  Type *ty;
  Token *name;
  int offset;
};

// AST Node kinds for Musab Language
typedef enum {
  ND_NUM,        // Numeric literal
  ND_STR,        // String literal
  ND_VAR,        // Variable
  ND_ADD,        // +
  ND_SUB,        // -
  ND_MUL,        // *
  ND_DIV,        // /
  ND_MOD,        // %
  ND_ASSIGN,     // =
  ND_EQ,         // ==
  ND_NE,         // !=
  ND_LT,         // <
  ND_LE,         // <=
  ND_GT,         // >
  ND_GE,         // >=
  ND_LOGAND,     // &&
  ND_LOGOR,      // ||
  ND_NOT,        // !
  ND_IF,         // if statement
  ND_WHILE,      // while loop
  ND_FOR,        // for loop
  ND_SWITCH,     // switch statement
  ND_CASE,       // case label
  ND_BREAK,      // break statement
  ND_CONTINUE,   // continue statement
  ND_RETURN,     // return statement
  ND_BLOCK,      // { ... }
  ND_FUNCALL,    // Function call
  ND_DECL,       // Variable/const declaration
} NodeKind;

// AST Node
struct Node {
  NodeKind kind;
  Node *next;
  Type *ty;
  Token *tok;
  
  // Binary operations
  Node *lhs;
  Node *rhs;
  
  // Control flow
  Node *cond;
  Node *then;
  Node *els;
  Node *body;
  Node *init;
  Node *inc;
  
  // Function call
  Node *args;
  char *func_name;
  
  // Variable/declaration
  char *name;
  bool is_const;
  
  // Numeric/string literal
  int64_t val;
  long double fval;
  char *str_val;
  
  // Case label
  int case_val;
  
  // Break/continue labels
  char *brk_label;
  char *cont_label;
};

// Variable/Function object
typedef struct Obj {
  struct Obj *next;
  char *name;
  Type *ty;
  Token *tok;
  bool is_local;
  bool is_function;
  bool is_const;
  int offset;           // For local variables
  struct Node *body;    // For functions
  struct Obj *params;   // For functions
  struct Obj *locals;   // For functions
  int stack_size;       // For functions
} Obj;

// Function declarations
noreturn void error(char *fmt, ...) __attribute__((format(printf, 1, 2)));
noreturn void error_at(char *loc, char *fmt, ...) __attribute__((format(printf, 2, 3)));
noreturn void error_tok(Token *tok, char *fmt, ...) __attribute__((format(printf, 2, 3)));
bool equal(Token *tok, char *op);
Token *skip(Token *tok, char *op);
bool consume(Token **rest, Token *tok, char *str);
Token *tokenize_file(char *filename);
Obj *parse(Token *tok);
void codegen(Obj *prog, FILE *out);
Type *new_type(TypeKind kind, int size, int align);
Type *pointer_to(Type *base);
Type *array_of(Type *base, int len);
void add_type(Node *node);
