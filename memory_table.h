#include <stdint.h>
#include <stdlib.h>

#ifndef MEMORY_TABLE
#define MEMORY_TABLE

#define TABLESIZE 4096
// equal to e.g. ~500 commands and ~2KB Memory Space hashed with no overlap

typedef struct memory_cell {
  uint64_t address;
  uint8_t content;
  struct memory_cell *next_cell;
} memory_cell;

typedef struct memory_table {
  memory_cell *memory[TABLESIZE];
  uint16_t initialised_cells;
} memory_table;

uint32_t hash(int64_t address);

memory_cell *create_memory_cell(uint64_t address, uint8_t content);
void add_memory_cell(memory_table *table, memory_cell *cell);

uint8_t get_memory_cell_content(
    memory_table *table,
    uint64_t address); // TODO: Random values for non initialised adresses?
                       // Currently returns 0 for them

void kill_memory_cell_recursive(
    memory_cell
        *cell); // Only call on cells directly in a memory_table as possible
                // next_cell-pointers to killed cell are not handled

memory_table *create_memory_table();
void kill_memory_table(memory_table *table);

uint64_t *get_initialised_adresses(
    memory_table *table); //[0] is length of the list, followed by the addresses

#endif // MEMORY_TABLE