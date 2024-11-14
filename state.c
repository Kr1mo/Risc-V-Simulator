#include "./state.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 32

// TODO: currently no checks if is initialised
uint8_t get_byte(state *s, uint64_t address) {return s->memory_values[address];
}
uint16_t get_halfword(state *s, uint64_t address) {
  uint16_t res = 0;
  res += s->memory_values[address + 1];
  res = res << 8;
  res += s->memory_values[address];
  return res;
}
uint32_t get_word(state *s, uint64_t address) {
  uint32_t res = 0;
  for (int8_t i = 3; i >= 0; i--) {
    res = res << 8;
    res += s->memory_values[i];
  }
  return res;
}
uint64_t get_doubleword(state *s, uint64_t address) {
  uint64_t res = 0;
  for (int8_t i = 7; i >= 0; i--) {
    res = res << 8;
    res += s->memory_values[i];
  }
  return res;
}
uint64_t get_register(state *s, uint8_t register_number) {
  return s->regs_values[register_number];
}
uint32_t get_next_command(state *s) { return get_word(s, s->pc); }

void set_byte(state *s, uint64_t address, uint8_t value) {
  s->memory_values[address] = value;
  s->memory_init[address] = true;
}
void set_halfword(state *s, uint64_t address, uint16_t value) {
  for (size_t i = 0; i < 2; i++) {
    s->memory_values[i] = (value >> (8 * i)) & 0xFF;
    s->memory_init[i] = true;
  }
}
void set_word(state *s, uint64_t address, uint32_t value) {
  for (size_t i = 0; i < 4; i++) {
    s->memory_values[i] = (value >> (8 * i)) & 0xFF;
    s->memory_init[i] = true;
  }
}
void set_doubleword(state *s, uint64_t address, uint64_t value) {
  for (size_t i = 0; i < 8; i++) {
    s->memory_values[i] = (value >> (8 * i)) & 0xFF;
    s->memory_init[i] = true;
  }
}
void set_register(state *s, uint8_t register_number, uint64_t value) {
  s->regs_values[register_number] = value;
  s->regs_init[register_number]=true;
}
void set_pc(state *s, uint64_t value) { s->pc = value; }
void next_pc(state *s) { s->pc += 4; }

char *byte_to_hex(char *dest, uint8_t b) {
  dest[0] = (b / 16) + 48;
  dest[1] = (b % 16) + 48;

  if (dest[0] > 57) {
    dest[0] = dest[0] + 39;
  }
  if (dest[1] > 57) {
    dest[1] = dest[1] + 39;
  }
  return dest;
}

bool pretty_print(state *s) {
  printf("Registers:\n");
  printf("PC:%ld\n", s->pc);
  for (size_t i = 0; i < 32; i++) {
    if (s->regs_init[i]) {
      printf("x%ld:%ld\n", i, s->regs_values[i]);
    }
  }

  printf("Memory:\n");

  char hex_str[3];
  hex_str[2] = '\0';
  for (size_t i = 0; i < MEMORY_SIZE; i = i + 4) {
    if (s->memory_init[i]) {
      byte_to_hex(hex_str, i);
      printf("%s: ", hex_str);
      for (int j = 3; j >= 0; j--) {

        byte_to_hex(hex_str, s->memory_values[i + j]);

        printf("%s ", hex_str);
      }
      printf("\n");
    }
  }
  return true;
}

bool is_address_initialised(state *s, uint64_t address) {
  return s->memory_init[address];
}
bool is_register_initialised(state *s, uint8_t register_number) {
  return s->regs_init[register_number];
}
bool is_next_command_initialised(state *s) {
  return is_address_initialised(s, s->pc) &&
         is_address_initialised(s, s->pc + 1) &&
         is_address_initialised(s, s->pc + 2) &&
         is_address_initialised(s, s->pc + 3);
}

state *create_new_state() {
  state *new = malloc(sizeof(state));
  new->pc = 0;

  new->regs_values[0] = 0;
  new->regs_init[0] = true;
  for (size_t i = 1; i < 32; i++) {
    new->regs_values[i] = 0;
    new->regs_init[i] = false;
  }

  for (size_t i = 0; i < MEMORY_SIZE; i++) {
    new->memory_init[i] = false;
    new->memory_values[i] = 0;
  }

  return new;
}

void remove_whitespace(char *str) {
  int8_t i = 0;
  int8_t offset = 0;
  char current_char = str[i];

  while (current_char != '\0' &&
         current_char !=
             '\n') // raw form of values will have /n, must be removed
  {
    if (current_char != ' ') // this char contains information
    {
      str[i - offset] = current_char; // move char to new position
    } else {
      offset++; // new space found, chars must be moved one space more to the
                // left
    }
    i++;
    current_char = str[i];
  }
  str[i - offset] =
      '\0'; // terminate string and cut off maybe left non moved chars
}

void remove_comment(char*str){
  for (size_t i = 0; i < BUFFER_SIZE; i++)
  {
    if (str[i] == '#')
    {
      str[i] = '\0';
    }
  }
}

bool load_state(char *filename, state *s) {
  FILE *state_file = fopen(filename, "r");
  if (!state_file) {
    printf("ERROR: No state-file");
    return false;
  }
  // file exists
  char buffer[BUFFER_SIZE]; // expected max size: 9 chars for mem_address in hex, 19
                   // chars for 64bit storage representation in hex, 4 chars for
                   // "; " and "/n/0"
  char *buffer_valid; // either NULL or buffer
  buffer_valid = fgets(buffer, sizeof(buffer), state_file);
  if (strcmp(buffer, "REGISTERS:\n") != 0) {
    printf("ERROR: state-file not starting with REGISTERS:");
    return false;
  }

  char *dp_pointer;
  char name_buffer[10];  // longest possible register will be ft11 (if
                         // implemented), 32bit address has 9 chars (2x4 hexcode + ' ')
  char value_buffer[22]; // All values, addresses,
  uint8_t name_length;

  buffer_valid = fgets(buffer, sizeof(buffer), state_file);
  remove_comment(buffer);

  while (buffer_valid && strncmp(buffer, "\n", 1)) {
    dp_pointer = strchr(buffer, ':');
    if (!dp_pointer) {
      buffer_valid = fgets(buffer, sizeof(buffer), state_file);
      remove_comment(buffer);
      continue;
    }

    name_length = dp_pointer - buffer;
    strncpy(name_buffer, buffer, name_length);
    name_buffer[name_length] = '\0';

    strcpy(value_buffer, dp_pointer + 1);

    remove_whitespace(name_buffer);
    remove_whitespace(value_buffer);

    switch (name_buffer[0]) {
    case 'P':
      s->pc = strtol(value_buffer, NULL, 16);
      break;

    case 'x':
      int16_t reg_num = strtol(name_buffer + 1, NULL, 10);
      if (reg_num > 31 || reg_num < 0) {
        printf("ERROR: register x%d out of range", reg_num);
        return false;
      } else if (reg_num == 0) {
        printf("ERROR: x0 is always 0");
        return false;
      }

      s->regs_values[reg_num] = strtol(value_buffer, NULL, 16);
      s->regs_init[reg_num] = true;
      break;

    default:
      printf("ERROR: register name %s unknown", name_buffer);
      break;
    }
    buffer_valid = fgets(buffer, sizeof(buffer), state_file);
    remove_comment(buffer);
  }

  buffer_valid = fgets(buffer, sizeof(buffer), state_file);
  remove_comment(buffer);
  if (strcmp(buffer, "MEMORY:\n") != 0) {
    printf("ERROR: state-file does not include MEMORY:");
    return false;
  }

  buffer_valid = fgets(buffer, sizeof(buffer), state_file);
  remove_comment(buffer);

  while (buffer_valid && strncmp(buffer, "\n", 1)) {
    dp_pointer = strchr(buffer, ':');
    if (!dp_pointer) {
      buffer_valid = fgets(buffer, sizeof(buffer), state_file);
      remove_comment(buffer);
      continue;
    }

    name_length = dp_pointer - buffer;
    strncpy(name_buffer, buffer, name_length);
    name_buffer[name_length] = '\0';

    strcpy(value_buffer, dp_pointer + 1);

    remove_whitespace(name_buffer);
    remove_whitespace(value_buffer);

    uint32_t address = strtol(name_buffer, NULL, 16);
    if (address >= MEMORY_SIZE) {
      printf("ERROR: memory address out of current bounds of %d", MEMORY_SIZE);
      return false;
    }

    int8_t next_byte_offset = strlen(value_buffer);
    if (next_byte_offset != 8 && next_byte_offset != 16) {
      printf("ERROR: Memory allocation neither 32 nor 64 bit");
      return false;
    }

    while (next_byte_offset) {
      next_byte_offset--;
      next_byte_offset--;
      s->memory_values[address] =
          strtol(value_buffer + next_byte_offset, NULL, 16);
      s->memory_init[address] = true;
      value_buffer[next_byte_offset] = '\0';
      address++;
    }

    buffer_valid = fgets(buffer, sizeof(buffer), state_file);
    remove_comment(buffer);
  }

  return true;
}

bool kill_state(state *s) {
  FILE *end_state = fopen("end.state", "w");
  fprintf(end_state, "REGISTERS:\n");
  fprintf(end_state, "PC:%lx", s->pc);
  for (size_t i = 0; i < 32; i++) {
    if (s->regs_init[i]) {
      fprintf(end_state, "x%ld:%lx\n", i, s->regs_values[i]);
    }
  }
  fprintf(end_state, "\nMEMORY:\n");
  char hex_str[3];
  hex_str[2] = '\0';
  for (size_t i = 0; i < MEMORY_SIZE; i = i + 4) {
    if (s->memory_init[i] || s->memory_init[i + 1] || s->memory_init[i + 2] ||
        s->memory_init[i + 3]) {
      byte_to_hex(hex_str, i);
      fprintf(end_state, "%s:", hex_str);

      for (int8_t j = 3; j < 0; j--) {
        byte_to_hex(hex_str, s->memory_values[i + j]);
        fprintf(end_state, "%s", hex_str);

        if (j == 1) {
          fprintf(end_state, " ");
        }
      }
      fprintf(end_state, "\n");
    }
  }

  fclose(end_state);
  free(s);

  return true;
}