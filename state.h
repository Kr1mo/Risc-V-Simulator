#include "./memory_table.h"
#include <stdbool.h>
#include <stdint.h>

#ifndef STATE
#define STATE

typedef struct state {
  uint64_t pc;
  uint64_t regs_values[32];
  bool regs_init[32];

  memory_table *memory;

} state;

uint8_t get_byte(state *s, uint64_t address);
uint16_t get_halfword(state *s, uint64_t address);
uint32_t get_word(state *s, uint64_t address);
uint64_t get_doubleword(state *s, uint64_t address);
uint64_t get_register(state *s, uint8_t register_number);
uint64_t get_pc(state *s);
uint32_t get_next_command(state *s, uint32_t hashed_first,
                          uint32_t hashed_second, uint32_t hashed_third,
                          uint32_t hashed_fourth);

void set_byte(state *s, uint64_t address, uint8_t value);
void set_halfword(state *s, uint64_t address, uint16_t value);
void set_word(state *s, uint64_t address, uint32_t value);
void set_doubleword(state *s, uint64_t address, uint64_t value);
void set_register(state *s, uint8_t register_number, uint64_t value);
void set_pc(state *s, uint64_t value);
void next_pc(state *s);

bool pretty_print(state *s);

bool is_address_initialised(state *s, uint64_t address,
                            uint32_t hashed_address);
bool is_register_initialised(state *s, uint8_t register_number);

state *create_new_state();

bool load_state(char *filename, state *s);

bool kill_state(state *s, char *filename);

#endif // STATE