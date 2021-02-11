#include "9cc.h"

int main(int argc, char **argv) {

  // check error
  if (argc != 2)
    error("%s: invalid number of arguments", argv[0]);

  // keep character_head
  user_input = argv[1];

  // tokenize
  token = tokenize();

  // parse
  Program *prog = program();

  // check stack_size
  int offset = 0;
  for (Var *var = prog->locals; var; var = var->next) {
    offset += 8;
    var->offset = offset;
  }
  prog->stack_size = offset;

  // generate code
  codegen(prog);

  return 0;
}

