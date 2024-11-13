#include "./state.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

  for (size_t i = 0; i < MEMORY_SIZE; i = i + 4) {
    if (s->memory_init[i]) {
      printf("%ld: ", i);
      for (int j = 3; j >= 0; j--) {
        char hex_str[3];
        hex_str[2] = '\0';

        byte_to_hex(hex_str, s->memory_values[i + j]);

        printf("%s ", hex_str);
      }
      printf("\n");
    }
  }
  return true;
}

bool is_adress_initialised(state *s, uint64_t adress) {
  return s->memory_init[adress];
}

bool is_register_initialised(state *s, uint8_t register_number) {
  return s->regs_init[register_number];
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

bool load_state(char *filename, state *s) {
  FILE *state_file = fopen(filename, "r");
  if (!state_file) {
    printf("ERROR: No state-file");
    return false;
  }
  // file exists
  char buffer[32]; // expected max size: 9 chars for mem_adress in hex, 19 chars
                   // for 64bit storage representation in hex, 4 chars for "; "
                   // and "/n/0"
  char *buffer_valid; // either NULL or buffer
  buffer_valid = fgets(buffer, sizeof(buffer), state_file);
  if (strcmp(buffer, "REGISTERS:\n") != 0) {
    printf("ERROR: state-file not starting with REGISTERS:");
    return false;
  }

  char *dp_pointer;
  char name_buffer[10];  // longest possible register will be ft11 (if
                         // implemented), 32bit adress has 9 chars
  char value_buffer[22]; // All values, adresses,
  int8_t name_length;

  buffer_valid = fgets(buffer, sizeof(buffer), state_file);

  while (buffer_valid && strncmp(buffer, "\n", 1)) {
    dp_pointer = strchr(buffer, ':');
    if (!dp_pointer) {
      buffer_valid = fgets(buffer, sizeof(buffer), state_file);
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
        printf("ERROR: cant write into x0");
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
  }

  buffer_valid = fgets(buffer, sizeof(buffer), state_file);
  if (strcmp(buffer, "MEMORY:\n") != 0) {
    printf("ERROR: state-file does not include MEMORY:");
    return false;
  }

  buffer_valid = fgets(buffer, sizeof(buffer), state_file);

  while (buffer_valid && strncmp(buffer, "\n", 1)) {
    dp_pointer = strchr(buffer, ':');
    if (!dp_pointer) {
      buffer_valid = fgets(buffer, sizeof(buffer), state_file);
      continue;
    }

    name_length = dp_pointer - buffer;
    strncpy(name_buffer, buffer, name_length);
    name_buffer[name_length] = '\0';

    strcpy(value_buffer, dp_pointer + 1);

    remove_whitespace(name_buffer);
    remove_whitespace(value_buffer);

    uint32_t adress = strtol(name_buffer, NULL, 16);
    if (adress >= MEMORY_SIZE) {
      printf("ERROR: memory adress out of current bounds of %d", MEMORY_SIZE);
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
      s->memory_values[adress] =
          strtol(value_buffer + next_byte_offset, NULL, 16);
      s->memory_init[adress] = true;
      value_buffer[next_byte_offset] = '\0';
      adress++;
    }

    buffer_valid = fgets(buffer, sizeof(buffer), state_file);
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
  for (size_t i = 0; i < MEMORY_SIZE; i = i + 4) {
    if (s->memory_init[i] || s->memory_init[i + 1] || s->memory_init[i + 2] ||
        s->memory_init[i + 3]) {
      fprintf(end_state, "%lx:", i);
      char hex_str[3];
      hex_str[2] = '\0';
      for (size_t j = 0; j < 4; j++) {
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