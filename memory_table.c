#include "memory_table.h"
#include <stdint.h>
#include <stdlib.h>

// Is called way to often, so hash will be available in the header

uint32_t
hash(int64_t address) { // copied the one_at_a_time hashing algorithm,
                        // seemed sufficient. Algorithm is
                        // modified to fit my case (eg removed while loop)
                        // https://en.wikipedia.org/wiki/Jenkins_hash_function
                        // 12.01.25
  uint32_t hash = 0;
  // This could have been a while loop but address size is fixed and hashing is
  // the most time intensive operation, so I take every bit of performance I can
  // get
  hash += address % 256; // use one byte per iteration
  hash += hash << 10;
  hash ^= hash >> 6;
  hash += (address >> (8)) % 256; // use one byte per iteration
  hash += hash << 10;
  hash ^= hash >> 6;
  hash += (address >> (16)) % 256; // use one byte per iteration
  hash += hash << 10;
  hash ^= hash >> 6;
  hash += (address >> (24)) % 256; // use one byte per iteration
  hash += hash << 10;
  hash ^= hash >> 6;
  hash += (address >> (32)) % 256; // use one byte per iteration
  hash += hash << 10;
  hash ^= hash >> 6;
  hash += (address >> (40)) % 256; // use one byte per iteration
  hash += hash << 10;
  hash ^= hash >> 6;
  hash += (address >> (48)) % 256; // use one byte per iteration
  hash += hash << 10;
  hash ^= hash >> 6;
  hash += (address >> (56)) % 256; // use one byte per iteration
  hash += hash << 10;
  hash ^= hash >> 6;
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
void set_memory_cell(memory_table *table, memory_cell *new_cell) {
  uint32_t location = hash(new_cell->address);
  memory_cell *cell_in_table = table->memory[location];
  if (!cell_in_table) {
    table->memory[location] = new_cell;
  } else {
    while (true) // Bad style, but found no limit that would actually trigger,
                 // so this feels 'honest'
    {
      if (cell_in_table->address == new_cell->address) // address match
      {
        cell_in_table->content = new_cell->content;
        free(new_cell);
        return; // do not increment initialised_cells as we freed the new cell
      } else if (!cell_in_table->next_cell) // no next cell AND no address match
      {
        cell_in_table->next_cell = new_cell;
        break; // needs to increment initialised_cells
      } else { // next_cell exists, so check next cell
        cell_in_table = cell_in_table->next_cell;
        continue; // repeat loop (keyword not needed but helps me understand the
                  // flow)
      }
    }
  }
  table->initialised_cells++;
}

void set_memory_cell_at_location(memory_table *table, memory_cell *new_cell,
                                 uint32_t location) {
  memory_cell *cell_in_table = table->memory[location];
  if (!cell_in_table) {
    table->memory[location] = new_cell;
  } else {
    while (true) // Bad style, but found no limit that would actually trigger,
                 // so this feels 'honest'
    {
      if (cell_in_table->address == new_cell->address) // address match
      {
        cell_in_table->content = new_cell->content;
        free(new_cell);
        return; // do not increment initialised_cells as we freed the new cell
      } else if (!cell_in_table->next_cell) // no next cell AND no address match
      {
        cell_in_table->next_cell = new_cell;
        break; // needs to increment initialised_cells
      } else { // next_cell exists, so check next cell
        cell_in_table = cell_in_table->next_cell;
        continue; // repeat loop (keyword not needed but helps me understand the
                  // flow)
      }
    }
  }
  table->initialised_cells++;
}

void set_memory(memory_table *table, uint64_t address, uint8_t content) {
  set_memory_cell(table, create_memory_cell(address, content));
}

void set_memory_at_location(memory_table *table, uint32_t location,
                            uint8_t content) {
  set_memory_cell_at_location(table, create_memory_cell(location, content),
                              location);
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

bool exists_address_in_table_at_location(memory_table *table, uint64_t address,
                                         uint32_t location) {
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

uint8_t get_memory_cell_content_at_location(
    memory_table *table, uint64_t address,
    uint32_t location) { // TODO: Random values for non initialised adresses?
                         // Currently returns 0 for them
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
int compare_alt(const void *a, const void *b) {
  return (*(uint64_t *)a > *(uint64_t *)b) - (*(uint64_t *)a < *(uint64_t *)b);
}

uint64_t *get_initialised_adresses(memory_table *table) {
  uint64_t *addresses =
      malloc(sizeof(uint64_t) *
             (table->initialised_cells + 1)); // +1 for length of the list
  addresses[0] = table->initialised_cells;
  uint32_t address_index = 1; // I do not expect more than 4.294.967.295 cells
                              // :)
  for (size_t i = 0; i < TABLESIZE; i++) {
    if (table->memory[i]) {
      memory_cell *previous = table->memory[i];
      addresses[address_index] = previous->address;
      address_index++;
      while (previous->next_cell) {
        previous = previous->next_cell;
        addresses[address_index] = previous->address;
        address_index++;
      }
    }
  }
  qsort(&addresses[1], addresses[0], sizeof(uint64_t), compare_alt);
  return addresses;
}
