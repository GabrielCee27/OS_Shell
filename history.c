#include "history.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//TODO
void print_history(struct history_entry **history, int c) {
  int i;

  //Need to set start and end point if count > HIST_MAX
  // End : c % HIST_MAX
  // start : c + 1

  for(i = 0; i < HIST_MAX && i < c; i++){
    printf("[%ld|%d:%d|%f] %s\n", history[i]->cmd_id, history[i]->hour, history[i]->min,
    history[i]->run_time, history[i]->command);
  }
}

struct history_entry *new_history_entry( int h, int m, int command_count, char *command_line, double exec_time) {
  struct history_entry * hist_ptr = malloc(sizeof(struct history_entry));

  //TODO: Populate vars
  hist_ptr->hour = h;
  hist_ptr->min = m;
  hist_ptr->cmd_id = command_count;
  hist_ptr->run_time = exec_time;
  strcpy(hist_ptr->command, command_line);

  return hist_ptr;
}
