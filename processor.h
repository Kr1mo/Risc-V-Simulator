#include "./state.h"

#ifndef PROCESSOR
#define PROCESSOR

#define OP_CODES_RV32I                                                         \
  55 /*LUI*/, 23 /*AUIPC*/, 111 /*JAL*/, 103 /*JALR*/, 99 /*Branch*/,          \
      3 /*Load*/, 35 /*Store*/, 19 /*Math-I*/, 51 /*Math*/, 15 /*FENCE*/,      \
      115 /*???*/
#define OP_CODES_RV64I                                                         \
  27 /*Math-I-W*/, 59 /*Math-W*/ // only what isnt already in RV32I

bool is_next_command_valid_opcode(state *s);

void execute_next_command(state *s);

#endif // PROCESSOR