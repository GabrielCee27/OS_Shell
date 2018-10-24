#include "history.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void debug_print_history(struct history_entry **history, int command_count){
  printf("debug print history: \n");
  int i;
  for(i = 0; i < HIST_MAX && i < command_count+1; i++){
    printf("i: %d [%ld|%d:%d|%f] %s\n", i, history[i]->cmd_id, history[i]->hour, history[i]->min,
    history[i]->run_time, history[i]->command);
  }

}

//BUG: Not printing out of order when exceeded HIST_MAX
void print_history(struct history_entry **history, int command_count) {
  // debug_print_history(history, command_count);

  int i = 0, count, elements = command_count + 1;

  if(command_count >= HIST_MAX){
    i = (command_count % HIST_MAX) + 1;
    elements = HIST_MAX;
  }

  // printf("Starting at: %d\n", i);
  // printf("Num of elemens: %d\n", elements);

  for(count = 0; count < elements; count++){
    if(i == HIST_MAX)
      i = 0;
    printf("[%ld|%d:%d|%f] %s\n", history[i]->cmd_id, history[i]->hour, history[i]->min,
    history[i]->run_time, history[i]->command);
    i++;
  }
}


struct history_entry *new_history_entry(int h, int m, int command_count, char *command_line,
  double exec_time) {
  struct history_entry * hist_ptr = malloc(sizeof(struct history_entry));

  hist_ptr->hour = h;
  hist_ptr->min = m;
  hist_ptr->cmd_id = command_count;
  hist_ptr->run_time = exec_time;
  strcpy(hist_ptr->command, command_line);

  return hist_ptr;
}

void overwrite_history_entry(struct history_entry *entry, int h, int m, int command_count, char *command_line,
  double exec_time){

    entry->hour = h;
    entry->min = m;
    entry->cmd_id = command_count;
    entry->run_time = exec_time;
    strcpy(entry->command, command_line);
  }

//TODO:
void get_command_at(int c_num, char *command, struct history_entry **history, int command_count){

  printf("Looking for command at: %d\n", c_num);
  printf("command_count: %d\n", command_count);
  debug_print_history(history, command_count);

  if(c_num > command_count || c_num < command_count - HIST_MAX){
    printf("The command you are trying to access is out of range!\n");
    //TODO: Error handle
    return;
  }

  printf("entry->cmd_id: %ld\n", history[c_num % HIST_MAX]->cmd_id);
  printf("entry->command: %s\n", history[c_num % HIST_MAX]->command);


}
