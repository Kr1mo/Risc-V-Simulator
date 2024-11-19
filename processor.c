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
  uint8_t rd = (command >> 7) % 32;
  uint8_t rs1 = (command >> 15) % 32;
  uint8_t rs2 = (command >> 20) % 32;
  uint64_t rs1_value = get_register(s, rs1);
  uint64_t rs2_value = get_register(s, rs2);
  switch ((command & 0x00007000) >> 12) // sort by funct3
  {
  case 0:                     // ADD or SUB
    if (command & 0x40000000) // is SUB
    {
      set_register(s, rd, rs1_value - rs2_value);
    } else // is ADD
    {
      set_register(s, rd, rs1_value + rs2_value);
    }
    break;

  case 1: // SLL
    set_register(s, rd,
                 rs1_value << (rs2_value & 0x3F)); // shift by lowest 6 bytes
    break;

  case 2: // SLT
    int64_t rs1_value_signed = rs1_value;
    int64_t rs2_value_signed = rs2_value;
    bool comparison_signed = rs1_value_signed < rs2_value_signed;
    set_register(s, rd, comparison_signed);
    break;

  case 3: // SLTU
    bool comparison_unsigned = rs1_value < rs2_value;
    set_register(s, rd, comparison_unsigned);
    break;

  case 4:                                       // XOR
    set_register(s, rd, rs1_value ^ rs2_value); // bitwise xor
    break;

  case 5: // SRL or SRA
    uint8_t shift_size = rs2_value & 0x3F;
    uint64_t shifted_value =
        rs1_value >> shift_size; // SRL & SRA with positive rs1
    if ((command & 0x40000000) &&
        (rs1_value & 0x8000000000000000)) { // SRA with negative rs1
      uint64_t mask =
          0xffffffffffffffff
          << (64 - shift_size); // generating a mask for sign extension
      shifted_value |= mask;
    }
    set_register(s, rd, shifted_value);
    break;

  case 6: // OR
    set_register(s, rd, rs1_value | rs2_value);
    break;

  case 7: // AND
    set_register(s, rd, rs1_value & rs2_value);
    break;

  default:
    printf(
        "ERROR: funct3 %u not yet implemented or this should be impossible\n",
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
