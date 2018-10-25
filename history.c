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

  int i = 0, count, elements = curr_cmd_id;

  if(curr_cmd_id >= HIST_MAX){
    i = (curr_cmd_id % HIST_MAX);
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
void get_command_at(int target_id, char *cmd_dest, struct history_entry **history, int curr_cmd_id){

  debug_print_history(history, curr_cmd_id);

  if(target_id >= curr_cmd_id || (target_id < curr_cmd_id - HIST_MAX && target_id >= 0)){
    printf("The command you are trying to access is out of range!\n");
    //TODO: Error handle
    return;
  }

  strcpy(cmd_dest, history[target_id % HIST_MAX]->command);
}

void get_last_cmd_of(char *target_cmd, char *cmd_dest, struct history_entry **history, int curr_cmd_id){
  //TODO
  //iterate through history backwards by cmd_id



}
