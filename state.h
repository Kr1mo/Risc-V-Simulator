#include <stdbool.h>
#include <stdint.h>

#ifndef STATE
#define STATE

#define MEMORY_SIZE 128

typedef struct state {
  uint64_t pc;
  uint64_t regs_values[32];
  bool regs_init[32];

  // TODO: Hashing
  uint8_t memory_values[MEMORY_SIZE];
  bool memory_init[MEMORY_SIZE];

} state;

bool pretty_print(state *s);

bool is_adress_initialised(state *s, uint64_t adress);

bool is_register_initialised(state *s, uint8_t register_number);

state *create_new_state();

bool load_state(char *filename, state *s);

bool kill_state(state *s);

#endif // STATE