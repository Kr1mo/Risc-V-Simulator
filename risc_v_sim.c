#include "./processor.h"
#include "./state.h"
#include <stdio.h>

int main(int argc, char **argv) {
  printf("generate new state\n");
  state *s = create_new_state();
  load_state("./test.state", s);
  pretty_print(s);
  execute_next_command(s);
  pretty_print(s);
  kill_state(s);
  return 42;
}
