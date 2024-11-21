#include "./processor.h"
#include "./state.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {
  char *source_state;
  char *end_state = NULL;
  uint16_t n_cycles = 1;
  bool debug = false;
  int opt;

  while ((opt = getopt(argc, argv, "c:e:d")) != -1) {
    switch (opt) {
    case 'n':
      n_cycles = strtol(optarg, NULL, 10);
      break;

    case 'e':
      char *file_extension = strrchr(optarg, '.');
      if (!file_extension) {
        printf("No file extension for end state\n");
        break;
      }
      if (!strcmp(file_extension, ".state")) {
        printf("Wrong file extension for end state, expected .state, got %s\n",
               file_extension);
        break;
      }
      end_state = optarg;
      break;

    case 'd':
      debug = true;

    case '?':
      if (optopt == 'c' || optopt == 'e')
        fprintf(stderr, "Option -%c requires an argument.\n", optopt);
      else
        fprintf(stderr, "Unknown option `-%c'.\n", optopt);
      return 1;
    default:
      abort();
    }
  }
  if (optind > argc) {
    printf("Expected path state file after options\n");
    return 1;
  }
  char *file_extension = strrchr(argv[optind], '.');
  if (!file_extension) {
    printf("No file extension for initial state\n");
    return 0;
  }
  if (strcmp(file_extension, ".state")) { // file extension is NOT .state
    printf("Wrong file extension for initial state, expected .state, got %s\n",
           file_extension);
    return 0;
  }
  source_state = argv[optind];

  state *s = create_new_state();
  load_state(source_state, s);
  pretty_print(s);
  if (is_next_command_initialised(s)) {
    execute_next_command(s);
    pretty_print(s);
  } else {
    printf("ERROR: Memory at PC is not initialised\n");
  }

  if (end_state) {
    kill_state(s, end_state);
  } else {
    kill_state(s, "states/end.state");
  }
  return 0;
}
