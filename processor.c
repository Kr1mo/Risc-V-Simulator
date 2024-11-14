#include "./processor.h"
#include "./state.h"
#include <stdbool.h>
#include <stdio.h>

bool is_next_command_valid_opcode(state *s) {
  uint8_t code = get_next_command(s) & 0x7F;
  uint8_t opcodes[] = {OP_CODES_RV32I, OP_CODES_RV64I};
  for (uint8_t i = 0; i < sizeof(opcodes); i++) {
    if (code == opcodes[i]) {
      return true;
    }
  }
  return false;
}

void execute_math_reg_only(state *s, uint32_t command) {
  switch (command & 0x7000) // sort by funct3
  {
  case 0:                     // ADD or SUB
    if (command & 0x40000000) // is SUB
    {
      // TODO
    } else // is ADD
    {
      uint8_t rd = (command >> 7) % 32;
      uint8_t rs1 = (command >> 15) % 32;
      uint8_t rs2 = (command >> 20) % 32;
      set_register(s, rd, get_register(s, rs1) + get_register(s, rs2));
    }

    break;

  default:
    printf("ERROR: funct3 %u not yet implemented or this should be impossible\n",
           command & 0x7000);
    break;
  }
}

void execute_next_command(state *s) {
  uint32_t command = get_next_command(s);
  switch (command & 0x7F) // sort by opcode
  {
  case 51: // Math
    execute_math_reg_only(s, command);
    break;

  default:
    printf("ERROR: opcode %u is unknown or not implemented yet\n",
           command & 0x7F);
    break;
  }
  next_pc(s);
}
