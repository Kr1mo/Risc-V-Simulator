#include "memory_table.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

uint32_t
hash(int64_t address) { // copied the one_at_a_time hashing algorithm,
                        // seemed sufficient. Algorithm is sligthly
                        // modified to fit my case
                        // https://en.wikipedia.org/wiki/Jenkins_hash_function
                        // 12.01.25
  uint8_t i = 0;
  uint32_t hash = 0;
  while (i < 8) {
    hash += (address >> (i * 8)) % 256; // use one byte per iteration
    hash += hash << 10;
    hash ^= hash >> 6;
    i++;
  }
  hash += hash << 3;
  hash ^= hash >> 11;
  hash += hash << 15;
  return hash % TABLESIZE;
}

memory_cell *create_memory_cell(uint64_t address, uint8_t content) {
  memory_cell *cell = malloc(sizeof(memory_cell));
  cell->address = address;
  cell->content = content;
  cell->next_cell = NULL;
  return cell;
}
void set_memory_cell(memory_table *table, memory_cell *cell) {
  uint32_t location = hash(cell->address);
  if (!table->memory[location]) {
    table->memory[location] = cell;
  } else {
    memory_cell *previous = table->memory[location];
    while (previous->next_cell) {
      if (cell->address == previous->address) {
        previous->content = cell->content;
        free(cell);
        return; // Don't increase initialised cells, number of cells did not
                // change
      }
      previous = previous->next_cell;
    }
    previous->next_cell = cell;
  }
  table->initialised_cells++;
}
void set_memory(memory_table *table, uint64_t address, uint8_t content) {
  set_memory_cell(table, create_memory_cell(address, content));
}
bool exists_address_in_table(memory_table *table, uint64_t address) {
  uint32_t location = hash(address);
  if (table->memory[location]) {
    memory_cell *previous = table->memory[location];
    while (previous) {
      if (previous->address == address) {
        return true;
      }
      previous = previous->next_cell;
    }
  }
  return false;
}

uint8_t get_memory_cell_content(
    memory_table *table,
    uint64_t address) { // TODO: Random values for non initialised adresses?
                        // Currently returns 0 for them
  uint32_t location = hash(address);
  if (!table->memory[location]) {
    return 0;
  }
  memory_cell *previous = table->memory[location];
  while (previous->address != address) {
    if (previous->next_cell) {
      previous = previous->next_cell;
    } else {
      return 0;
    }
  }
  return previous->content;
}

void kill_memory_cell_recursive(
    memory_cell
        *cell) // Only call on cells directly in a memory_table as possible
               // next_cell-pointers to killed cell are not handleds
{
  if (cell->next_cell) {
    kill_memory_cell_recursive(cell->next_cell);
  }
  free(cell);
}
memory_table *create_memory_table() {
  memory_table *table = malloc(sizeof(memory_table));
  table->initialised_cells = 0;
  for (size_t i = 0; i < TABLESIZE; i++) {
    table->memory[i] = NULL;
  }
  return table;
}
void kill_memory_table(memory_table *table) {
  for (size_t i = 0; i < TABLESIZE; i++) {
    if (table->memory[i]) {
      kill_memory_cell_recursive(table->memory[i]);
    }
  }
  free(table);
}

int compare(const void *a, const void *b) { return (*(int *)a - *(int *)b); }
int compare_alt(const void *a, const void *b){
  return (*(uint64_t*)a > *(uint64_t*)b) - (*(uint64_t*)a < *(uint64_t*)b);
  }

uint64_t *get_initialised_adresses(memory_table *table) {
  int8_t deepness = 0;
  int64_t overall = 0;
  int64_t over = 0;
  int64_t under = 0;
  int64_t average_chainig = table->initialised_cells / TABLESIZE;
  int64_t tolerated_over = average_chainig + 5;
  int64_t tolerated_under = average_chainig - 5;
  uint64_t *addresses =
      malloc(sizeof(uint64_t) *
             (table->initialised_cells + 1)); // +1 for length of the list
  addresses[0] = table->initialised_cells;
  uint32_t address_index = 1; //I do not expect more than 4.294.967.295 cells :)
  for (size_t i = 0; i < TABLESIZE; i++) {
    deepness = 0;
    if (table->memory[i]) {
      memory_cell *previous = table->memory[i];
      addresses[address_index] = previous->address;
      address_index++;
      deepness++;
      while (previous->next_cell) {
        previous = previous->next_cell;
        addresses[address_index] = previous->address;
        address_index++;
        deepness++;
      }
    }
    if (deepness>=tolerated_over)
    {
      printf("\x1b[31m""%d""\x1b[0m"",", deepness); //in red
      over++;
    }else if (deepness>tolerated_under) //in tolerated margin
    {
      printf("\x1b[32m""%d""\x1b[0m"",", deepness); //in green
    }
    else
    {
      printf("\x1b[34m""%d""\x1b[0m"",", deepness); //in blue
      under++;
    }
    overall += deepness;
  }
printf("\nOverall counted %ld of %ld cells\nAt least 5 over expected: %ld, under: %ld\n", overall, table->initialised_cells, over, under);
  qsort(&addresses[1], addresses[0], sizeof(uint64_t), compare_alt);
  return addresses;
}
