#include "./processor.h"
#include "./state.h"
#include <stdbool.h>

bool is_opcode(uint8_t code) {
  int8_t opcodes[] = {OP_CODES_RV32I, OP_CODES_RV64I};
  for (uint8_t i = 0; i < sizeof(opcodes); i++) {
    if (code == opcodes[i]) {
      return true;
    }
  }
  return false;
}

state *execute_next_command(state *s);

bool is_next_command_valid_opcode(state *s);
