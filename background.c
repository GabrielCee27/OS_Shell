#include "background.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct background_entry *new_background_entry (int p_id, char *cmd_ln, double start_time) {

  struct background_entry *bg_ptr = malloc(sizeof(struct background_entry));

  bg_ptr->pid = p_id;
  strcpy(bg_ptr->cmd, cmd_ln);
  bg_ptr->start = start_time;

  return bg_ptr;
}
