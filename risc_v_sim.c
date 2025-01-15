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
  int n_cycles = 1;
  bool keep_going_until_not_initialised = false;
  bool debug = false;
  int opt;

  while ((opt = getopt(argc, argv, "n:e:d")) != -1) {
    switch (opt) {
    case 'c':
      n_cycles = strtol(optarg, NULL, 10);
      if (n_cycles == -1) {
        keep_going_until_not_initialised = true;
      } else if (n_cycles < 0) // only -1 starts "perpetual" execution, other
                               // negative values are ignored
      {
        n_cycles = 0;
      }
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
  if (debug) {
    pretty_print(s);
  }
  size_t executed_cycles = 0;
  while (is_next_command_initialised(s) &&
         (executed_cycles < n_cycles || keep_going_until_not_initialised)) {
    execute_next_command(s);
    if (debug) {
      pretty_print(s);
    }
    executed_cycles++;
  }
  if (keep_going_until_not_initialised) {
    printf("Ended execution after %ld cycles\n", executed_cycles);
  } else if (executed_cycles < n_cycles) {
    printf("ERROR: Memory at PC %ld is not initialised\n", get_pc(s));
  }

  for (size_t i = 0; i < n_cycles; i++) {
    if (is_next_command_initialised(s)) {
      execute_next_command(s);
      if (debug) {
        pretty_print(s);
      }
    } else {
      printf("ERROR: Memory at PC %ld is not initialised\n", get_pc(s));
      break;
    }
  }

  printf("\n\nLast State:\n");
  pretty_print(s);

  if (end_state) {
    kill_state(s, end_state);
  } else {
    kill_state(s, "states/end.state");
  }
  return 0;
}
