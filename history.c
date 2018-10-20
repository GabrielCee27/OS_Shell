#include "history.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_history(struct history_entry **history, int command_count) {

  int i = 0, count, elements = command_count + 1;

  if(command_count >= HIST_MAX){
    i = (command_count % HIST_MAX) + 1;
    elements = HIST_MAX;
  }

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
