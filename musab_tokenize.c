#include "musab_lang.h"

// Tokenizer for Musab Language
// Converts source code into a sequence of tokens

static File *current_file;
static Token *token_head;

void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  va_end(ap);
  exit(1);
}

void error_at(char *loc, char *fmt, ...) {
  fprintf(stderr, "%s: ", current_file->name);
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  va_end(ap);
  exit(1);
}

void error_tok(Token *tok, char *fmt, ...) {
  fprintf(stderr, "%s:%d: ", tok->filename, tok->line_no);
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  va_end(ap);
  exit(1);
}

bool equal(Token *tok, char *op) {
  return tok->kind == TK_PUNCT && tok->len == strlen(op) &&
         strncmp(tok->loc, op, tok->len) == 0;
}

Token *skip(Token *tok, char *op) {
  if (!equal(tok, op))
    error_tok(tok, "expected '%s'", op);
  return tok->next;
}

bool consume(Token **rest, Token *tok, char *str) {
  if (equal(tok, str)) {
    *rest = tok->next;
    return true;
  }
  *rest = tok;
  return false;
}

static Token *new_token(TokenKind kind, char *start, char *end) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->loc = start;
  tok->len = end - start;
  tok->file = current_file;
  tok->filename = current_file->name;
  tok->line_no = 1;
  return tok;
}

static bool is_alpha(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static bool is_digit(char c) {
  return c >= '0' && c <= '9';
}

static bool is_alnum(char c) {
  return is_alpha(c) || is_digit(c);
}

static KeywordType is_keyword(Token *tok) {
  if (tok->kind != TK_IDENT) return KW_NONE;
  
  char *word = tok->loc;
  int len = tok->len;
  
  if (len == 3 && strncmp(word, "var", 3) == 0) return KW_VAR;
  if (len == 5 && strncmp(word, "const", 5) == 0) return KW_CONST;
  if (len == 2 && strncmp(word, "fn", 2) == 0) return KW_FN;
  if (len == 6 && strncmp(word, "struct", 6) == 0) return KW_STRUCT;
  if (len == 4 && strncmp(word, "enum", 4) == 0) return KW_ENUM;
  if (len == 3 && strncmp(word, "use", 3) == 0) return KW_USE;
  if (len == 2 && strncmp(word, "if", 2) == 0) return KW_IF;
  if (len == 4 && strncmp(word, "else", 4) == 0) return KW_ELSE;
  if (len == 6 && strncmp(word, "switch", 6) == 0) return KW_SWITCH;
  if (len == 4 && strncmp(word, "case", 4) == 0) return KW_CASE;
  if (len == 7 && strncmp(word, "default", 7) == 0) return KW_DEFAULT;
  if (len == 5 && strncmp(word, "while", 5) == 0) return KW_WHILE;
  if (len == 3 && strncmp(word, "for", 3) == 0) return KW_FOR;
  if (len == 5 && strncmp(word, "break", 5) == 0) return KW_BREAK;
  if (len == 8 && strncmp(word, "continue", 8) == 0) return KW_CONTINUE;
  if (len == 6 && strncmp(word, "return", 6) == 0) return KW_RETURN;
  
  return KW_NONE;
}

static Token *read_string_literal(char *start) {
  char *p = start + 1;  // Skip opening quote
  char buf[256] = {};
  int len = 0;
  
  while (*p != '"' && *p != '\0') {
    if (*p == '\\') {
      p++;
      if (*p == 'n') buf[len++] = '\n';
      else if (*p == 't') buf[len++] = '\t';
      else if (*p == 'r') buf[len++] = '\r';
      else buf[len++] = *p;
    } else {
      buf[len++] = *p;
    }
    p++;
  }
  
  if (*p != '"')
    error_at(start, "unclosed string literal");
  
  Token *tok = new_token(TK_STR, start, p + 1);
  tok->str = calloc(1, len + 1);
  strncpy(tok->str, buf, len);
  return tok;
}

static Token *read_number(char *start) {
  char *p = start;
  bool is_float = false;
  
  // Read integer part
  while (is_digit(*p)) p++;
  
  // Check for float
  if (*p == '.' || *p == 'e' || *p == 'E') {
    is_float = true;
    if (*p == '.') {
      p++;
      while (is_digit(*p)) p++;
    }
    if (*p == 'e' || *p == 'E') {
      p++;
      if (*p == '+' || *p == '-') p++;
      while (is_digit(*p)) p++;
    }
  }
  
  Token *tok = new_token(TK_NUM, start, p);
  if (is_float) {
    tok->fval = strtold(start, NULL);
  } else {
    tok->val = strtoll(start, NULL, 10);
  }
  return tok;
}

static Token *read_ident(char *start) {
  char *p = start;
  while (is_alnum(*p)) p++;
  return new_token(TK_IDENT, start, p);
}

Token *tokenize_file(char *filename) {
  FILE *fp = fopen(filename, "r");
  if (!fp)
    error("cannot open %s", filename);
  
  // Read file contents
  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);
  rewind(fp);
  
  char *contents = calloc(1, size + 1);
  fread(contents, 1, size, fp);
  fclose(fp);
  
  current_file = calloc(1, sizeof(File));
  current_file->name = filename;
  current_file->contents = contents;
  current_file->file_no = 0;
  
  Token head = {};
  Token *cur = &head;
  char *p = contents;
  int line_no = 1;
  
  while (*p) {
    // Skip whitespace
    if (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') {
      if (*p == '\n') line_no++;
      p++;
      continue;
    }
    
    // Skip comments
    if (*p == '/' && p[1] == '/') {
      while (*p && *p != '\n') p++;
      continue;
    }
    
    // String literals
    if (*p == '"') {
      Token *tok = read_string_literal(p);
      tok->line_no = line_no;
      cur = cur->next = tok;
      p += tok->len;
      continue;
    }
    
    // Numbers
    if (is_digit(*p)) {
      Token *tok = read_number(p);
      tok->line_no = line_no;
      cur = cur->next = tok;
      p += tok->len;
      continue;
    }
    
    // Identifiers and keywords
    if (is_alpha(*p)) {
      Token *tok = read_ident(p);
      tok->line_no = line_no;
      
      // Check if it's a keyword
      KeywordType kw = is_keyword(tok);
      if (kw != KW_NONE) {
        tok->kind = TK_KEYWORD;
      }
      
      cur = cur->next = tok;
      p += tok->len;
      continue;
    }
    
    // Punctuators
    char *start = p;
    if (*p == ':' && p[1] == '=') {
      p += 2;
    } else if (*p == '=' && p[1] == '=') {
      p += 2;
    } else if (*p == '!' && p[1] == '=') {
      p += 2;
    } else if (*p == '<' && p[1] == '=') {
      p += 2;
    } else if (*p == '>' && p[1] == '=') {
      p += 2;
    } else if (*p == '&' && p[1] == '&') {
      p += 2;
    } else if (*p == '|' && p[1] == '|') {
      p += 2;
    } else {
      p++;
    }
    
    Token *tok = new_token(TK_PUNCT, start, p);
    tok->line_no = line_no;
    cur = cur->next = tok;
  }
  
  // Add EOF token
  Token *eof = new_token(TK_EOF, p, p);
  eof->line_no = line_no;
  cur = cur->next = eof;
  
  return head.next;
}
