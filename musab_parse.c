#include "musab_lang.h"
#include <string.h>

// Parser for Musab Language
// Builds Abstract Syntax Tree (AST) from tokens

static Token *current_token;
static Obj *globals = NULL;
static Obj *locals = NULL;

static Node *new_node(NodeKind kind, Token *tok) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->tok = tok;
  return node;
}

static Node *new_binary(NodeKind kind, Node *lhs, Node *rhs, Token *tok) {
  Node *node = new_node(kind, tok);
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

static Node *new_unary(NodeKind kind, Node *expr, Token *tok) {
  Node *node = new_node(kind, tok);
  node->lhs = expr;
  return node;
}

static Node *new_num(int64_t val, Token *tok) {
  Node *node = new_node(ND_NUM, tok);
  node->val = val;
  return node;
}

static Node *new_str(char *str, Token *tok) {
  Node *node = new_node(ND_STR, tok);
  node->str_val = str;
  return node;
}

static Obj *new_var(char *name, Type *ty) {
  Obj *obj = calloc(1, sizeof(Obj));
  obj->name = name;
  obj->ty = ty;
  return obj;
}

static Obj *find_var(char *name) {
  // Search in local scope
  for (Obj *obj = locals; obj; obj = obj->next) {
    if (strcmp(obj->name, name) == 0) {
      return obj;
    }
  }
  
  // Search in global scope
  for (Obj *obj = globals; obj; obj = obj->next) {
    if (strcmp(obj->name, name) == 0) {
      return obj;
    }
  }
  
  return NULL;
}

static Token *peek(void) {
  return current_token;
}

static bool check(TokenKind kind) {
  return current_token->kind == kind;
}

static bool check_kw(KeywordType kw) {
  if (current_token->kind != TK_KEYWORD) return false;
  
  Token *tok = current_token;
  if (kw == KW_VAR && tok->len == 3 && strncmp(tok->loc, "var", 3) == 0) return true;
  if (kw == KW_CONST && tok->len == 5 && strncmp(tok->loc, "const", 5) == 0) return true;
  if (kw == KW_FN && tok->len == 2 && strncmp(tok->loc, "fn", 2) == 0) return true;
  if (kw == KW_IF && tok->len == 2 && strncmp(tok->loc, "if", 2) == 0) return true;
  if (kw == KW_ELSE && tok->len == 4 && strncmp(tok->loc, "else", 4) == 0) return true;
  if (kw == KW_RETURN && tok->len == 6 && strncmp(tok->loc, "return", 6) == 0) return true;
  if (kw == KW_WHILE && tok->len == 5 && strncmp(tok->loc, "while", 5) == 0) return true;
  if (kw == KW_FOR && tok->len == 3 && strncmp(tok->loc, "for", 3) == 0) return true;
  if (kw == KW_SWITCH && tok->len == 6 && strncmp(tok->loc, "switch", 6) == 0) return true;
  if (kw == KW_CASE && tok->len == 4 && strncmp(tok->loc, "case", 4) == 0) return true;
  if (kw == KW_BREAK && tok->len == 5 && strncmp(tok->loc, "break", 5) == 0) return true;
  if (kw == KW_CONTINUE && tok->len == 8 && strncmp(tok->loc, "continue", 8) == 0) return true;
  
  return false;
}

static Token *advance(void) {
  Token *tok = current_token;
  if (current_token->kind != TK_EOF) {
    current_token = current_token->next;
  }
  return tok;
}

static char *get_ident(void) {
  if (current_token->kind != TK_IDENT) {
    error_tok(current_token, "expected identifier");
  }
  
  char *name = calloc(1, current_token->len + 1);
  strncpy(name, current_token->loc, current_token->len);
  advance();
  return name;
}

static Type *parse_type(void) {
  if (current_token->kind == TK_IDENT) {
    char *type_name = calloc(1, current_token->len + 1);
    strncpy(type_name, current_token->loc, current_token->len);
    advance();
    
    if (strcmp(type_name, "int") == 0) {
      return get_int_type();
    } else if (strcmp(type_name, "float") == 0) {
      return get_float_type();
    } else if (strcmp(type_name, "string") == 0) {
      return get_string_type();
    } else if (strcmp(type_name, "bool") == 0) {
      return get_bool_type();
    } else if (strcmp(type_name, "void") == 0) {
      return get_void_type();
    }
    error("unknown type: %s", type_name);
  }
  error_tok(current_token, "expected type name");
  return NULL;
}

// Forward declarations
static Node *expr(void);
static Node *stmt(void);

static Node *primary(void) {
  // Number
  if (check(TK_NUM)) {
    Token *tok = advance();
    return new_num(tok->val, tok);
  }
  
  // String
  if (check(TK_STR)) {
    Token *tok = advance();
    return new_str(tok->str, tok);
  }
  
  // Identifier or function call
  if (check(TK_IDENT)) {
    Token *ident_tok = advance();
    char *name = calloc(1, ident_tok->len + 1);
    strncpy(name, ident_tok->loc, ident_tok->len);
    
    // Function call
    if (equal(current_token, "(")) {
      advance();  // consume (
      
      Node *call = new_node(ND_FUNCALL, ident_tok);
      call->func_name = name;
      
      // Parse arguments
      Node *args = NULL;
      Node *arg_tail = NULL;
      
      while (!equal(current_token, ")")) {
        Node *arg = expr();
        if (!args) {
          args = arg;
          arg_tail = arg;
        } else {
          arg_tail->next = arg;
          arg_tail = arg;
        }
        
        if (equal(current_token, ",")) {
          advance();
        }
      }
      
      skip(current_token, ")");
      call->args = args;
      return call;
    }
    
    // Variable reference
    Node *var = new_node(ND_VAR, ident_tok);
    var->name = name;
    return var;
  }
  
  // Parenthesized expression
  if (equal(current_token, "(")) {
    advance();
    Node *node = expr();
    skip(current_token, ")");
    return node;
  }
  
  error_tok(current_token, "unexpected token");
  return NULL;
}

static Node *mul(void) {
  Node *node = primary();
  
  while (equal(current_token, "*") || equal(current_token, "/") || equal(current_token, "%")) {
    Token *tok = advance();
    
    if (equal(tok, "*")) {
      node = new_binary(ND_MUL, node, primary(), tok);
    } else if (equal(tok, "/")) {
      node = new_binary(ND_DIV, node, primary(), tok);
    } else {
      node = new_binary(ND_MOD, node, primary(), tok);
    }
  }
  
  return node;
}

static Node *add(void) {
  Node *node = mul();
  
  while (equal(current_token, "+") || equal(current_token, "-")) {
    Token *tok = advance();
    
    if (equal(tok, "+")) {
      node = new_binary(ND_ADD, node, mul(), tok);
    } else {
      node = new_binary(ND_SUB, node, mul(), tok);
    }
  }
  
  return node;
}

static Node *compare(void) {
  Node *node = add();
  
  while (equal(current_token, "<") || equal(current_token, "<=") ||
         equal(current_token, ">") || equal(current_token, ">=") ||
         equal(current_token, "==") || equal(current_token, "!=")) {
    Token *tok = advance();
    
    if (equal(tok, "<")) {
      node = new_binary(ND_LT, node, add(), tok);
    } else if (equal(tok, "<=")) {
      node = new_binary(ND_LE, node, add(), tok);
    } else if (equal(tok, ">")) {
      node = new_binary(ND_GT, node, add(), tok);
    } else if (equal(tok, ">=")) {
      node = new_binary(ND_GE, node, add(), tok);
    } else if (equal(tok, "==")) {
      node = new_binary(ND_EQ, node, add(), tok);
    } else {
      node = new_binary(ND_NE, node, add(), tok);
    }
  }
  
  return node;
}

static Node *and(void) {
  Node *node = compare();
  
  while (equal(current_token, "&&")) {
    Token *tok = advance();
    node = new_binary(ND_LOGAND, node, compare(), tok);
  }
  
  return node;
}

static Node *or(void) {
  Node *node = and();
  
  while (equal(current_token, "||")) {
    Token *tok = advance();
    node = new_binary(ND_LOGOR, node, and(), tok);
  }
  
  return node;
}

static Node *expr(void) {
  return or();
}

static Node *stmt(void) {
  // Return statement
  if (check_kw(KW_RETURN)) {
    advance();
    Node *node = new_node(ND_RETURN, current_token - 1);
    node->lhs = expr();
    skip(current_token, ";");
    return node;
  }
  
  // If statement
  if (check_kw(KW_IF)) {
    advance();
    skip(current_token, "(");
    
    Node *node = new_node(ND_IF, current_token - 1);
    node->cond = expr();
    
    skip(current_token, ")");
    skip(current_token, "{");
    
    node->then = stmt();
    
    skip(current_token, "}");
    
    if (check_kw(KW_ELSE)) {
      advance();
      skip(current_token, "{");
      node->els = stmt();
      skip(current_token, "}");
    }
    
    return node;
  }
  
  // While loop
  if (check_kw(KW_WHILE)) {
    advance();
    skip(current_token, "(");
    
    Node *node = new_node(ND_WHILE, current_token - 1);
    node->cond = expr();
    
    skip(current_token, ")");
    skip(current_token, "{");
    
    node->body = stmt();
    
    skip(current_token, "}");
    
    return node;
  }
  
  // Expression statement
  Node *node = expr();
  skip(current_token, ";");
  return node;
}

Obj *parse(Token *tok) {
  current_token = tok;
  
  // Parse program structure
  while (current_token->kind != TK_EOF) {
    // use statement
    if (check_kw(KW_USE)) {
      advance();
      get_ident();  // Skip library name
      skip(current_token, ";");
      continue;
    }
    
    // Variable declaration
    if (check_kw(KW_VAR) || check_kw(KW_CONST)) {
      bool is_const = check_kw(KW_CONST);
      advance();
      
      char *name = get_ident();
      skip(current_token, ":");
      Type *ty = parse_type();
      
      Obj *var = new_var(name, ty);
      var->is_const = is_const;
      var->next = globals;
      globals = var;
      
      if (equal(current_token, "=")) {
        advance();
        expr();
      }
      skip(current_token, ";");
      continue;
    }
    
    // Function declaration
    if (check_kw(KW_FN)) {
      advance();
      
      char *func_name = get_ident();
      skip(current_token, "(");
      
      Obj *func = new_var(func_name, func_type(get_void_type()));
      func->is_function = true;
      func->next = globals;
      globals = func;
      
      // Skip parameters for now
      while (!equal(current_token, ")")) {
        advance();
      }
      skip(current_token, ")");
      
      // Check for return type (optional)
      if (equal(current_token, ":")) {
        advance();
        parse_type();  // return type - just consume it for now
      }
      
      // Function body
      if (equal(current_token, "{")) {
        advance();
        
        // Skip function body
        int brace_count = 1;
        while (brace_count > 0 && current_token->kind != TK_EOF) {
          if (equal(current_token, "{")) brace_count++;
          if (equal(current_token, "}")) brace_count--;
          advance();
        }
      } else {
        error_tok(current_token, "expected '{' after function declaration");
      }
      
      continue;
    }
    
    advance();
  }
  
  return globals;
}
