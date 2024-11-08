#include <stdio.h>
#include "./state.h"
#include "./processor.h"

int main(int argc, char **argv) {
  printf("generate new state\n");
  state* s = create_new_state();
  load_state("./test.state", s);
  return 42;
}
