#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "./state.h"

bool pretty_print(state* s);

bool is_adress_initialised(state* s, int64_t adress);

bool is_register_initialised(state* s, int8_t register_number);

state* create_new_state(){
    state* new = malloc(sizeof(state));
    new->pc = 0;

    new->regs_values[0] = 0;
    new->regs_init[0] = true;
    for(size_t i = 1;i<32;i++){
        new->regs_values[i]=0;
        new->regs_init[i]=false;
    }
    
    for (size_t i = 0; i < MEMORY_SIZE; i++)
    {
        new->memory_init[i] = false;
        new->memory_values[i] = 0;
    }
    
    return new;
}

bool load_state(char* filename, state* s){
    FILE* state_file = fopen(filename, "r");
    if(!state_file){
        printf("ERROR: No state-file");
        return false;
    }
    // file exists
    char buffer[16];
    fgets(buffer, sizeof(buffer), state_file);
    if(strcmp(buffer, "REGISTERS:\n") != 0){
        printf("ERROR: state-file not starting with REGISTERS:");
        return false;
    }
    int64_t* value = malloc(sizeof(int64_t));
    int loopstop = 20;
    while(fscanf(state_file, "%s: %ln\n", buffer, value) && loopstop){
        printf("Register %s steht auf %ln\n", buffer, value);
        loopstop--;
    }
    return true;
}