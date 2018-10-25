#include "history.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void debug_print_history(struct history_entry **history, int curr_cmd_id){
  printf("debug print history: \n");
  int i;
  for(i = 0; i < HIST_MAX && i < curr_cmd_id; i++){
    printf("i: %d [%ld|%d:%d|%f] %s\n", i, history[i]->cmd_id, history[i]->hour, history[i]->min,
    history[i]->run_time, history[i]->command);
  }

}

//BUG: Not printing out of order when exceeded HIST_MAX
void print_history(struct history_entry **history, int curr_cmd_id) {
  // debug_print_history(history, curr_cmd_id);

  int i = 0, count, elements = curr_cmd_id + 1;

  if(curr_cmd_id >= HIST_MAX){
    i = (curr_cmd_id % HIST_MAX) + 1;
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


struct history_entry *new_history_entry(int h, int m, int curr_cmd_id, char *command_line,
  double exec_time) {
  struct history_entry * hist_ptr = malloc(sizeof(struct history_entry));

  hist_ptr->hour = h;
  hist_ptr->min = m;
  hist_ptr->cmd_id = curr_cmd_id;
  hist_ptr->run_time = exec_time;
  strcpy(hist_ptr->command, command_line);

  return hist_ptr;
}

void overwrite_history_entry(struct history_entry *entry, int h, int m, int curr_cmd_id, char *command_line,
  double exec_time){

    entry->hour = h;
    entry->min = m;
    entry->cmd_id = curr_cmd_id;
    entry->run_time = exec_time;
    strcpy(entry->command, command_line);
  }

//TODO:
void get_command_at(int target_id, char *command_src, struct history_entry **history, int curr_cmd_id){

  printf("Looking for command w/ id: %d\n", target_id);
  printf("curr_cmd_id: %d\n", curr_cmd_id);
  debug_print_history(history, curr_cmd_id);

  if(target_id >= curr_cmd_id || (target_id < curr_cmd_id - HIST_MAX && target_id >= 0)){
    printf("The command you are trying to access is out of range!\n");
    //TODO: Error handle
    return;
  }

  printf("entry->cmd_id: %ld\n", history[target_id % HIST_MAX]->cmd_id);
  printf("entry->command: %s\n", history[target_id % HIST_MAX]->command);
  strcpy(command_src, history[target_id % HIST_MAX]->command);
}
