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


void remove_whitespace(char* str){
    int8_t i = 0;
    int8_t offset = 0;
    char current_char = str[i];

    while (current_char != '\0' && current_char != '\n') // raw form of values will have /n, must be removed
    {
        if (current_char != ' ') //this char contains information
        {
            str[i - offset] = current_char; //move char to new position
        }
        else
        {
            offset++; //new space found, chars must be moved one space more to the left
        }
        i++;
        current_char = str[i];
    }
    str[i-offset] = '\0'; //terminate string and cut off maybe left non moved chars
}


bool load_state(char* filename, state* s){
    FILE* state_file = fopen(filename, "r");
    if(!state_file){
        printf("ERROR: No state-file");
        return false;
    }
    // file exists
    char buffer [32]; //expected max size: 9 chars for mem_adress in hex, 19 chars for 64bit storage representation in hex, 4 chars for "; " and "/n/0"
    char* buffer_valid; //either NULL or buffer
    buffer_valid = fgets(buffer, sizeof(buffer), state_file);
    if(strcmp(buffer, "REGISTERS:\n") != 0){
        printf("ERROR: state-file not starting with REGISTERS:");
        return false;
    }

    char* dp_pointer;
    char name_buffer[10]; //longest possible register will be ft11 (if implemented), 32bit adress has 9 chars
    char value_buffer[22]; //All values, adresses, 
    int8_t name_length;

    buffer_valid = fgets(buffer, sizeof(buffer), state_file);

    while(buffer_valid && strncmp(buffer, "\n", 1)){
        dp_pointer = strchr(buffer, ':');
        if (!dp_pointer)
        {
            buffer_valid = fgets(buffer, sizeof(buffer), state_file);
            continue;
        }
        
        name_length = dp_pointer - buffer;
        strncpy(name_buffer, buffer, name_length);
        name_buffer[name_length] = '\0';

        strcpy(value_buffer, dp_pointer + 1);
        
        remove_whitespace(name_buffer);
        remove_whitespace(value_buffer);
        
        switch (name_buffer[0])
        {
        case 'P':
            s->pc = strtol(value_buffer, NULL, 16);
            break;
        
        case 'x':
            int16_t reg_num = strtol(value_buffer, NULL, 10);
            if (reg_num > 31 || reg_num < 0)
            {
                printf("ERROR: register x%n out of range", reg_num);
                return false;
            }
            s->memory_values[reg_num] = strtol(value_buffer, NULL, 16);
            s->memory_init[reg_num] = true;
            break;

        default:
            printf("ERROR: register name %s unknown", name_buffer);
            break;
        }

        printf("Register %s steht auf %s\n", name_buffer, value_buffer);
        buffer_valid = fgets(buffer, sizeof(buffer), state_file);
    }

    buffer_valid = fgets(buffer, sizeof(buffer), state_file);
    if(strcmp(buffer, "MEMORY:\n") != 0){
        printf("ERROR: state-file does not include MEMORY:");
        return false;
    }

    buffer_valid = fgets(buffer, sizeof(buffer), state_file);

    while(buffer_valid && strncmp(buffer, "\n", 1)){
        dp_pointer = strchr(buffer, ':');
        if (!dp_pointer)
        {
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
        if (adress >= MEMORY_SIZE)
        {
            printf("ERROR: memory adress out of current bounds of %n", MEMORY_SIZE);
            return false;
        }

        int8_t next_byte_offset = strlen(value_buffer);
        if (next_byte_offset != 8 && next_byte_offset != 16)
        {
            printf("ERROR: Memory allocation neither 32 nor 64 bit");
            return false;
        }        

        while (next_byte_offset);
        {
            next_byte_offset--;
            next_byte_offset--;
            s->memory_values[adress] = strtol(value_buffer + next_byte_offset, NULL, 16);
            value_buffer[next_byte_offset] = '\0';
            adress++;            

        printf("Adresse %s steht auf %s\n", name_buffer, value_buffer);
        }

        
        buffer_valid = fgets(buffer, sizeof(buffer), state_file);
    }

    return true;
}