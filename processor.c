#include "./processor.h"
#include "./state.h"
#include <stdbool.h>
#include <stdio.h>

void execute_math_reg_only(state *s, uint32_t command) {
  uint8_t rd = (command >> 7) % 32;
  uint64_t rs1_value = get_register(s, (command >> 15) % 32);
  uint64_t rs2_value = get_register(s, (command >> 20) % 32);
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
    printf("ERROR: funct3 %u for math_reg_only not yet implemented or this "
           "should be impossible\n",
           command & 0x7000);
    break;
  }

  next_pc(s);
}

void execute_math_immediate(state *s, uint32_t command) {
  int64_t immediate = 0;
  if (command & 0x80000000) // immediate is negative
  {
    immediate |= 0xFFFFFFFFFFFFF800; // sign extend immediate
  }
  immediate |=
      (command & 0xFFF00000) >> 20; // immediate[11:0] is at command[31:20]
  uint64_t rs1_value = get_register(s, (command >> 15) % 32);
  uint8_t rd = (command >> 7) % 32;

  switch ((command & 0x00007000) >> 12) // sort by funct3
  {
  case 0: // ADDI
    set_register(s, rd, rs1_value + immediate);
    break;
  case 1: // SLLI
    if (immediate >> 6) {
      printf("ERROR: immediate %ld unfitting for SLLI", immediate);
    }
    set_register(s, rd, rs1_value << (immediate % 64));
    break;
  case 2: // SLTI
    int64_t rs1_value_signed = rs1_value;
    bool lt_signed = rs1_value_signed < immediate;
    set_register(s, rd, lt_signed);
    break;
  case 3: // SLTIU
    uint64_t immediate_unsigned = immediate;
    bool lt_unsigned = rs1_value < immediate_unsigned;
    set_register(s, rd, lt_unsigned);
    break;
  case 4: // XORI
    set_register(s, rd, rs1_value ^ immediate);
    break;
  case 5: // SRLI and SRAI
    if ((immediate >> 6) != 0 && immediate >> 6 != 0x10) {
      printf("ERROR: immediate %ld unfitting for SRLI or SRAI\n", immediate);
    }
    uint8_t shift_size = immediate % 64;
    uint64_t shifted_value =
        rs1_value >> shift_size; // SRL & SRA with positive rs1
    if ((command & 0x40000000) &&
        (rs1_value & 0x8000000000000000)) { // SRAI with negative rs1
      uint64_t mask =
          0xffffffffffffffff
          << (64 - shift_size); // generating a mask for sign extension
      shifted_value |= mask;
    }
    set_register(s, rd, shifted_value);
    break;
  case 6: // ORI
    set_register(s, rd, rs1_value | immediate);
    break;
  case 7: // ANDI
    set_register(s, rd, rs1_value & immediate);
    break;
  default:
  }

  next_pc(s);
}

void execute_store(state *s, uint32_t command) {
  int16_t offset = 0;       // offset is 12bit and split into 2 parts
  if (command & 0x80000000) // offset is negative
  {
    offset |= 0xFFFFFFFFFFFFF000; // sign extend offset
  }
  offset |= (command & 0xFE000000) >> 20; // offset[11:5] is at command[31:25]
  offset |= (command >> 7) % 32;          // offset[4:0] is at command[11:7]
  uint64_t rs1_value = get_register(s, (command >> 15) % 32);
  uint64_t rs2_value = get_register(s, (command >> 20) % 32);
  uint64_t address = rs1_value + offset;
  switch ((command & 0x00007000) >> 12) // sort by funct3
  {
  case 0: // Byte
    uint8_t byte = (rs2_value << 56) >> 56;
    set_byte(s, address, byte);
    break;

  case 1: // Halfword
    uint16_t halfword = (rs2_value << 48) >> 48;
    set_halfword(s, address, halfword);
    break;

  case 2: // Word
    uint32_t word = (rs2_value << 32) >> 32;
    set_word(s, address, word);
    break;

  case 3: // Doubleword
    set_doubleword(s, address, rs2_value);
    break;

  default:
    printf("ERROR: funct3 %u for execute_store not intended\n",
           command & 0x7000);
  }

  next_pc(s);
}

void execute_load(state *s, uint32_t command) {
  int16_t immediate = 0;
  if (command & 0x80000000) // immediate is negative
  {
    immediate |= 0xFFFFFFFFFFFFF800; // sign extend immediate
  }
  immediate |=
      (command & 0xFFF00000) >> 20; // immediate[11:0] is at command[31:20]
  uint64_t rs1_value = get_register(s, (command >> 15) % 32);
  uint64_t address = rs1_value + immediate;
  uint8_t rd = (command >> 7) % 32;

  uint64_t result = 0;

  switch ((command & 0x00007000) >> 12) // sort by funct3
  {
  case 0: // Byte
    uint8_t b = get_byte(s, address);
    if (b & 0x80) // negative
    {
      result |= 0xFFFFFFFFFFFFFF00; // sign extend
    }
    result |= b;
    break;

  case 1: // Halfword
    uint16_t hw = get_halfword(s, address);
    if (hw & 0x8000) // negative
    {
      result |= 0xFFFFFFFFFFFF0000; // sign extend
    }
    result |= hw;
    break;

  case 2: // Word
    uint32_t w = get_word(s, address);
    if (w & 0x80000000) // negative
    {
      result |= 0xFFFFFFFF00000000; // sign extend
    }
    result |= w;
    break;

  case 3: // Doubleword
    result = get_doubleword(s, address);
    break;

  case 4: // Byte unsigned
    result |= get_byte(s, address);
    break;

  case 5: // Halfword unsigned
    result |= get_halfword(s, address);
    break;

  case 6: // Word unsigned
    result |= get_word(s, address);
    break;
  }
  set_register(s, rd, result);
  next_pc(s);
}

void execute_branch(state *s, uint32_t command) {
  int16_t offset = 0; // there are 4 parts of the 12bit offset in the command, I
                      // add them successively
  if (command & 0x80000000) // offset is negative
  {
    offset |= 0xFFFFFFFFFFFFF000; // sign extend offset
  }
  offset |= (command & 0x7E000000) >> 20; // offset[10:5] is at command[30:25]
  offset |= (command & 0x00000F00) >> 7;  // offset[4:1] is at command[11:8]
  offset |= (command & 0x00000080) << 4;  // offset[11] is at command[7]

  uint64_t rs1_value = get_register(s, (command >> 15) % 32);
  uint64_t rs2_value = get_register(s, (command >> 20) % 32);
  int64_t rs1_value_signed = rs1_value;
  int64_t rs2_value_signed = rs2_value;

  bool flag = false;
  switch ((command & 0x00007000) >> 12) // sort by funct3
  {
  case 0: // BEQ
    flag = rs1_value == rs2_value;
    break;
  case 1: // BNE
    flag = rs1_value != rs2_value;
    break;
  case 4: // BLT
    flag = rs1_value_signed < rs2_value_signed;
    break;
  case 5: // BGE
    flag = rs1_value_signed >= rs2_value_signed;
    break;
  case 6: // BLTU
    flag = rs1_value < rs2_value;
    break;
  case 7: // BGEU
    flag = rs1_value >= rs2_value;
    break;
  default:
    printf("ERROR: funct3 %u for branch not implemented or does not exist\n",
           command & 0x7000);
  }
  if (flag) {
    set_pc(s, s->pc + offset);
  } else {
    next_pc(s);
  }
}

void execute_jal(state *s, uint32_t command) {
  int32_t offset = 0;
  if (command & 0x80000000) // offset is negative
  {
    offset |= 0xFFFFFFFFFFF00000; // sign extend offset and set highest bit,
                                  // offset is 21bit
  }
  offset |= (command & 0x7FE00000) >> 20; // offset[10:1] is at command[30:21]
  offset |= (command & 0x00100000) >> 9;  // offset[11] is at command[20]
  offset |= (command & 0x000FF000);       // offset[19:12] is at command[19:12]
  uint8_t rd = (command >> 7) % 32;
  if (rd) {
    set_register(s, rd, s->pc + 4);
  }
  set_pc(s, s->pc+ offset);
}

void execute_jalr(
    state *s,
    uint32_t command) { // TODO: add Return-Address prediciton Stack ???
  int16_t offset = 0;
  if (command & 0x80000000) // offset is negative
  {
    offset |= 0xFFFFFFFFFFFFF000; // sign extend offset and set highest bit,
                                  // offset is 21bit
  }
  offset |= (command & 0xFFF00000) >> 20; // offset[11:0] is at command[31:20]
  uint8_t rd = (command >> 7) % 32;
  uint64_t rs1_value = get_register(s, (command >> 15) % 32);

  if (rd) {
    set_register(s, rd, s->pc + 4);
  }
  set_pc(s, offset + rs1_value);
}

void execute_lui(state *s, uint32_t command) {
  uint64_t immediate = 0;
  if (command & 0x80000000) // offset is negative
  {
    immediate |= 0xFFFFFFFF00000000; // sign extend offset and set highest bit,
                                     // offset is 21bit
  }
  immediate |= (command & 0xFFFFF000); // immediate[31:12] is at command[31:12]
  uint8_t rd = (command >> 7) % 32;
  set_register(s, rd, immediate);
  next_pc(s);
}

void execute_auipc(state *s, uint32_t command) {
  uint64_t immediate = 0;
  if (command & 0x80000000) // offset is negative
  {
    immediate |= 0xFFFFFFFF00000000; // sign extend offset and set highest bit,
                                     // offset is 21bit
  }
  immediate |= (command & 0xFFFFF000); // immediate[31:12] is at command[31:12]
  uint8_t rd = (command >> 7) % 32;
  set_register(s, rd, immediate + s->pc);
  next_pc(s);
}

void execute_math_w(state *s, uint32_t command) {
  uint8_t rd = (command >> 7) % 32;
  uint32_t rs1_value_shortend = get_register(s, (command >> 15) % 32);
  uint32_t rs2_value_shortend = get_register(s, (command >> 20) % 32);
  uint64_t result = 0;

  switch ((command & 0x00007000) >> 12) // sort by funct3
  {
  case 0:                     // ADDW or SUBW
    if (command & 0x40000000) // SUBW
    {
      result = rs1_value_shortend - rs2_value_shortend;
    } else // ADDW
    {
      result = rs1_value_shortend + rs2_value_shortend;
    }
    break;
  case 1:                                                       // SLLW
    result = rs1_value_shortend << (rs2_value_shortend & 0x3F); // lowest 6bit
  case 5:                                                       // SRLW or SRAW
    uint8_t shift_size = rs2_value_shortend & 0x3F;
    uint32_t shifted_value =
        rs1_value_shortend >> shift_size; // SRL & SRA with positive rs1
    if ((command & 0x40000000) &&
        (rs1_value_shortend & 0x80000000)) { // SRA with negative rs1
      uint64_t mask = 0xffffffff
                      << (32 -
                          shift_size); // generating a mask for sign extension
      shifted_value |= mask;
    }
    result = shifted_value;
    break;
  default:
    printf("ERROR: funct3 %u for math_w not yet implemented or not defined\n",
           command & 0x7000);
  }
  if (result & 0x0000000080000000) // word is negative, sign extension needed
  {
    result |= 0xffffffff00000000;
  }
  set_register(s, rd, result);
  next_pc(s);
}

void execute_math_w_immediate(state *s, uint32_t command) {
  uint8_t rd = (command >> 7) % 32;
  uint32_t rs1_value_shortend = get_register(s, (command >> 15) % 32);
  int16_t immediate = (command & 0xFFF00000) >> 20;
  if (immediate & 0x0800) {
    immediate |= 0xF000;
  }

  uint64_t result = 0;

  switch ((command & 0x00007000) >> 12) // sort by funct3
  {
  case 0: // ADDIW
    result = rs1_value_shortend + immediate;
    break;
  case 1:                                              // SLLW
    result = rs1_value_shortend << (immediate & 0x3F); // lowest 6bit
    break;
  case 5: // SRLW or SRAW
    uint8_t shift_size = immediate & 0x3F;
    uint32_t shifted_value =
        rs1_value_shortend >> shift_size; // SRL & SRA with positive rs1
    if ((command & 0x40000000) &&
        (rs1_value_shortend & 0x80000000)) { // SRA with negative rs1
      uint64_t mask = 0xffffffff
                      << (32 -
                          shift_size); // generating a mask for sign extension
      shifted_value |= mask;
    }
    result = shifted_value;
    break;
  default:
    printf("ERROR: funct3 %u for math_w_immediate not yet implemented or not "
           "defined\n",
           command & 0x7000);
  }
  if (result & 0x0000000080000000) // word is negative, sign extension needed
  {
    result |= 0xffffffff00000000;
  }
  set_register(s, rd, result);
  next_pc(s);
}

void execute_next_command(state *s, uint32_t hashed_first,
                          uint32_t hashed_second, uint32_t hashed_third,
                          uint32_t hashed_fourth) {
  uint32_t command = get_next_command(s, hashed_first, hashed_second,
                                      hashed_third, hashed_fourth);
  switch (command & 0x7F) // sort by opcode
  {
  case 51: // 0110011b Math register only
    execute_math_reg_only(s, command);
    break;

  case 19: // 0010011b Math immediate
    execute_math_immediate(s, command);
    break;

  case 35: // 0100011b Store
    execute_store(s, command);
    break;

  case 3: // 0000011b Load
    execute_load(s, command);
    break;

  case 99: // 1100011b Branch
    execute_branch(s, command);
    break;

  case 111: // 1101111b Jump
    execute_jal(s, command);
    break;

  case 103: // 1100111b Jump relative
    execute_jalr(s, command);
    break;

  case 55: // 0110111b LUI
    execute_lui(s, command);
    break;

  case 23: // 0010111b AUIPC
    execute_auipc(s, command);
    break;

  case 59: // 0111011b Math word
    execute_math_w(s, command);
    break;

  case 27: // 0011011b Math word immediate
    execute_math_w_immediate(s, command);
    break;

  default:
    printf("ERROR: opcode %u is unknown or not implemented yet, PC was "
           "increased.\n",
           command & 0x7F);
    next_pc(s);
    break;
  }
}
