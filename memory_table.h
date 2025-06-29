#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef MEMORY_TABLE
#define MEMORY_TABLE

#define TABLESIZE 4096
// equal to at most ~500 commands and ~2KB Memory Space hashed without overlap

typedef struct memory_cell {
  uint64_t address;
  uint8_t content;
  struct memory_cell *next_cell;
} memory_cell;

typedef struct memory_table {
  memory_cell *memory[TABLESIZE];
  uint64_t initialised_cells;
} memory_table;

uint32_t hash(int64_t address);

memory_cell *create_memory_cell(uint64_t address, uint8_t content);
void set_memory_cell(memory_table *table, memory_cell *cell);
void set_memory_at_location(memory_table *table, uint32_t location,
                            uint8_t content);
void set_memory(memory_table *table, uint64_t address, uint8_t content);
void set_memory_at_location(memory_table *table, uint32_t location,
                            uint8_t content);
bool exists_address_in_table(memory_table *table, uint64_t address);
bool exists_address_in_table_at_location(memory_table *table, uint64_t address,
                                         uint32_t location);

uint8_t get_memory_cell_content(
    memory_table *table,
    uint64_t address); // TODO: Random values for non initialised adresses?
                       // Currently returns 0 for them
uint8_t get_memory_cell_content_at_location(memory_table *table,
                                            uint64_t address,
                                            uint32_t location);

void kill_memory_cell_recursive(
    memory_cell
        *cell); // Only call on cells directly in a memory_table as possible
                // next_cell-pointers to killed cell are not handled

memory_table *create_memory_table();
void kill_memory_table(memory_table *table);

uint64_t *get_initialised_adresses(
    memory_table *table); //[0] is length of the list, followed by the addresses

#endif // MEMORY_TABLE