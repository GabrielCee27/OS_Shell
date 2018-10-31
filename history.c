#include "history.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void debug_print_history(struct history_entry **history, int curr_cmd_id){
  printf("debug print history: \n");
  int i;
  for(i = 0; i < HIST_MAX && i < curr_cmd_id; i++){
    printf("i: %d | pid: %d [%ld|%d:%d|%f] %s\n", i, history[i]->pid, history[i]->cmd_id, history[i]->hour, history[i]->min,
    history[i]->run_time, history[i]->command);
  }

}

void print_history(struct history_entry **history, int curr_cmd_id) {
  int i = 0, count, num_entries = curr_cmd_id;

  if(curr_cmd_id >= HIST_MAX){
    i = (curr_cmd_id % HIST_MAX);
    num_entries = HIST_MAX;
  }

  for(count = 0; count < num_entries; count++){
    if(i == HIST_MAX)
      i = 0;
    printf("[%ld|%d:%d|%f] %s\n", history[i]->cmd_id, history[i]->hour, history[i]->min,
    history[i]->run_time, history[i]->command);
    i++;
  }
}


struct history_entry *new_history_entry(int p_id, int h, int m, int curr_cmd_id, char *command_line,
  double exec_time) {
  struct history_entry * hist_ptr = malloc(sizeof(struct history_entry));

  hist_ptr->pid = p_id;
  hist_ptr->hour = h;
  hist_ptr->min = m;
  hist_ptr->cmd_id = curr_cmd_id;
  hist_ptr->run_time = exec_time;
  strcpy(hist_ptr->command, command_line);

  return hist_ptr;
}

void overwrite_history_entry(struct history_entry *entry, int p_id, int h, int m, int curr_cmd_id, char *command_line,
  double exec_time){

    entry->pid = p_id;
    entry->hour = h;
    entry->min = m;
    entry->cmd_id = curr_cmd_id;
    entry->run_time = exec_time;
    strcpy(entry->command, command_line);
  }

void get_command_at(int target_id, char *cmd_dest, struct history_entry **history, int curr_cmd_id){
  if(target_id >= curr_cmd_id || (target_id < curr_cmd_id - HIST_MAX && target_id >= 0)){
    printf("The command you are trying to access is out of range!\n");
    //TODO: Error handle
    return;
  }
  strcpy(cmd_dest, history[target_id % HIST_MAX]->command);
}

void get_last_cmd_of(char *target_cmd, struct history_entry **history, int curr_cmd_id){
  int i = curr_cmd_id - 1, count, num_entries = curr_cmd_id;

  if(curr_cmd_id >= HIST_MAX){
    i = (curr_cmd_id % HIST_MAX) - 1; //start at last entry
    num_entries = HIST_MAX;
  }

  for(count = 0; count < num_entries; count++){
    if(i == -1)
      i = HIST_MAX - 1;

    int target_len = strlen(target_cmd);
    if(strncmp(target_cmd, history[i]->command, target_len-1) == 0){
      strcpy(target_cmd, history[i]->command);
      return;
    }
    i--;
  }

  printf("Did not find command in history\n");
  strcpy(target_cmd, ""); //blank should be handled in shell

}
