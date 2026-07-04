#include "musab_lang.h"
#include <string.h>

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <file.ms>\n", argv[0]);
    return 1;
  }

  char *filename = argv[1];
  
  // Tokenize
  Token *tokens = tokenize_file(filename);
  if (!tokens) {
    error("Failed to tokenize file");
    return 1;
  }
  
  // Parse
  Obj *prog = parse(tokens);
  if (!prog) {
    error("Failed to parse file");
    return 1;
  }
  
  // Generate code
  FILE *out = stdout;
  codegen(prog, out);
  
  return 0;
}
